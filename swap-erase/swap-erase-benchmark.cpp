#include <vector>
#include <random>
#include <numeric>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <functional>

// A large object, for demonstration purposes.
struct NotSmallObject {
    // Vector of contiguous numbers.
    std::vector<std::size_t> v;

    // Vector size.
    std::size_t size;

    NotSmallObject() = default;

    NotSmallObject(std::mt19937& mt, std::size_t sz) {
        // Get the size of the vector between 0 and sz.
        std::uniform_int_distribution<std::size_t> dist(0u, sz);
        size = dist(mt);
        v = std::vector<std::size_t>(size);
        // Fill the vector with numbers from 0 to sz - 1.
        std::iota(v.begin(), v.end(), 0u);
    }

    // Check if an object is "smaller than n", i.e. its
    // vector has fewer than n elements. This is considered
    // as a quick check function.
    bool is_smaller_than(std::size_t n) const {
        return size < n;
    }

    // Check if the sum of the numbers in the vector is
    // smaller than n. This is considered as a slower check
    // function.
    bool is_sum_smaller_than(std::size_t n) const {
        return std::accumulate(v.begin(), v.end(), 0u) < n;
    }
};

// The function we want to test: erase element from a vector of objects
// if they fulfill a certain condition. It does not guarantee that the
// remaining elements are preserved in the same order as they were.
template<typename Object, typename Condition>
void swap_erase(std::vector<Object>& v, const Condition& condition) {
    // Keeps track to one past the last element we want to keep.
    auto iter_to_last = v.end();

    for(auto it = v.begin(); it < iter_to_last; ++it) {
        // If the erasure condition is fulfilled...
        if(condition(*it)) {
            // Increase by one to the left the "tail" of the
            // vector, made by elements we want to get rid of;
            // Swap the two elements.
            // Rewind the current iterator by 1, so at the
            // next iteration we test the element we just swapped.
            std::iter_swap(it--, --iter_to_last);
        }
    }

    // Erase the elements we pushed at the end of the queue.
    v.erase(iter_to_last, v.end());
}

// Same as above, but with move instead of swap.
template<typename Object, typename Condition>
void move_erase(std::vector<Object>& v, const Condition& condition) {
    auto iter_to_last = v.end();

    for(auto it = v.begin(); it < iter_to_last; ++it) {
        if(condition(*it)) {
            *it = std::move(*(--iter_to_last));
            --it;
        }
    }

    v.erase(iter_to_last, v.end());
}

void test(std::size_t obj_sz, std::size_t vec_sz, std::string fn) {
    using namespace std;
    using namespace std::chrono;

    random_device rd;
    mt19937 mt(rd());

    // We can run two types of test: this one is supposedly very quick...
    auto is_small = [&obj_sz] (const NotSmallObject& obj) {
        return obj.is_smaller_than(obj_sz / 10);
    };
    // ...and this one slower, as it first have to sum the numbers in the vectors.
    auto has_small_sum = [&obj_sz] (const NotSmallObject& obj) {
        return obj.is_sum_smaller_than((obj_sz * (obj_sz - 1)) / 20);
    };
    std::function<bool(const NotSmallObject&)> f;

    // Which test does the user want?
    if(fn == "quicker") {
        f = is_small;
    } else if(fn == "slower") {
        f = has_small_sum;
    } else {
        _Exit(1);
    }

    // Create the vectors to use for testing.
    vector<NotSmallObject> v1(vec_sz);
    generate(v1.begin(), v1.end(), [&mt, &obj_sz] () { return move(NotSmallObject(mt, obj_sz)); });
    auto v2 = v1;
    auto v3 = v1;

    auto s1 = high_resolution_clock::now();

    // First test: remove-erase idiom.
    v1.erase(std::remove_if(v1.begin(), v1.end(), f), v1.end());

    printf("%10s, %10zu, %10zu, ", fn.c_str(), obj_sz, vec_sz);

    auto e1 = high_resolution_clock::now();
    auto t1 = duration_cast<duration<float>>(e1 - s1).count();

    printf("%10.6f, ", t1);

    auto s2 = high_resolution_clock::now();

    // Second test: our swap-erase function.
    swap_erase(v2, f);

    auto e2 = high_resolution_clock::now();
    auto t2 = duration_cast<duration<float>>(e2 - s2).count();

    printf("%10.6f, %10.6f, ", t2, t1/t2);

    auto s3 = high_resolution_clock::now();

    // Third test: our move-erase function.
    move_erase(v3, f);

    auto e3 = high_resolution_clock::now();
    auto t3 = duration_cast<duration<float>>(e3 - s3).count();

    printf("%10.6f, %10.6f\n", t3, t1/t3);
}

int main() {
    // Sizes to use for the test. The first one is the max size of the vector inside the object,
    // telling us how heavy each object is. The second one is the size of the vector of objects
    // we are erasing from.
    std::vector<std::pair<std::size_t, std::size_t>> vo = {
        {10,     10000}, {100,    10000}, {1000,    10000}, {10000,   10000}, {100000, 10000},
        {10,    100000}, {100,   100000}, {1000,   100000}, {10000,  100000},
        {10,   1000000}, {100,  1000000}, {1000,  1000000},
        {10,  10000000}, {100, 10000000}
    };

    // For each combination, test both with quicker and the slower functions.
    for(const auto& [obj_sz, vec_sz] : vo) {
        test(obj_sz, vec_sz, "quicker");
        test(obj_sz, vec_sz, "slower");
    }

    return 0;
}