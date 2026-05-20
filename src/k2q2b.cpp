#include <chrono>
#include <iostream>
#include <random>
#include <sycl/sycl.hpp>
#include <thread>
#include <vector>

int VectorAdd(sycl::queue &q1, sycl::queue &q2, std::vector<int> &a,
              std::vector<int> &b, int iter, int array_size) {

  sycl::buffer a_buf(a);
  sycl::buffer b_buf(b);
  sycl::buffer<int> *sum_buf[2 * iter];
  for (size_t i = 0; i < (2 * iter); i++)
    sum_buf[i] = new sycl::buffer<int>(256);

  size_t num_groups = 1;
  size_t wg_size = 256;
  auto start = std::chrono::steady_clock::now();
  for (int i = 0; i < iter; i++) {
    q1.submit([&](auto &h) {
      sycl::accessor a_acc(a_buf, h, sycl::read_only);
      sycl::accessor b_acc(b_buf, h, sycl::read_only);
      auto sum_acc = sum_buf[2 * i]->get_access<sycl::access::mode::write>(h);

      h.parallel_for(sycl::nd_range<1>(num_groups * wg_size, wg_size),
                     [=](sycl::nd_item<1> index) {
                       size_t loc_id = index.get_local_id();
                       sum_acc[loc_id] = 0;
                       for (size_t i = loc_id; i < array_size; i += wg_size) {
                         sum_acc[loc_id] += a_acc[i] + b_acc[i];
                       }
                     });
    });
    q2.submit([&](auto &h) {
      sycl::accessor a_acc(a_buf, h, sycl::read_only);
      sycl::accessor b_acc(b_buf, h, sycl::read_only);
      auto sum_acc =
          sum_buf[2 * i + 1]->get_access<sycl::access::mode::write>(h);

      h.parallel_for(sycl::nd_range<1>(num_groups * wg_size, wg_size),
                     [=](sycl::nd_item<1> index) {
                       size_t loc_id = index.get_local_id();
                       sum_acc[loc_id] = 0;
                       for (size_t i = loc_id; i < array_size; i += wg_size) {
                         sum_acc[loc_id] += a_acc[i] + b_acc[i];
                       }
                     });
    });
  }
  q1.wait();
  q2.wait();
  auto end = std::chrono::steady_clock::now();
  std::cout << "Vector add completed on device - took " << (end - start).count()
            << " u-secs\n";
  // check results
  for (size_t i = 0; i < (2 * iter); i++)
    delete sum_buf[i];
  return ((end - start).count());
} // end VectorAdd

int main() {
  try {
    // Create SYCL queues
    sycl::property_list p{sycl::property::queue::enable_profiling()};
    sycl::queue q1(sycl::default_selector_v, p);
    sycl::queue q2(q1.get_context(), sycl::default_selector_v, p);

    int array_size = 100000;
    int iter = 100;

    // Print device information
    std::cout << "Running on device: "
              << q1.get_device().get_info<sycl::info::device::name>()
              << std::endl;

    // Create random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(1, 100);

    // Generate random input arrays
    std::vector<int> a(array_size);
    std::vector<int> b(array_size);

    std::cout << "Generating random arrays of size " << array_size << std::endl;

    for (size_t i = 0; i < array_size; i++) {
      a[i] = dis(gen);
      b[i] = dis(gen);
    }

    std::cout << "Starting vector addition with " << iter << " iterations..."
              << std::endl;

    // Call the VectorAdd function
    int execution_time = VectorAdd(q1, q2, a, b, iter, array_size);

    std::cout << "Total execution time: " << execution_time << " microseconds"
              << std::endl;

    // Print first few elements for verification
    std::cout << "\nFirst 5 elements of array a: ";
    for (int i = 0; i < 5 && i < array_size; i++) {
      std::cout << a[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "First 5 elements of array b: ";
    for (int i = 0; i < 5 && i < array_size; i++) {
      std::cout << b[i] << " ";
    }
    std::cout << std::endl;

  } catch (const sycl::exception &e) {
    std::cerr << "SYCL exception caught: " << e.what() << std::endl;
    return 1;
  } catch (const std::exception &e) {
    std::cerr << "Standard exception caught: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
