#include <iostream>
#include <string>
#include <typeinfo>
#include <cxxabi.h>

std::string classname(const std::type_info& ti) {
    int status;
    return abi::__cxa_demangle(ti.name(), 0, 0, &status);
}

struct Point {
    double x;
    double y;
};

struct Level0 {
    double operator()(const Point& p) const {
        return p.x * p.y;
    }
};

template<typename L0>
struct Level1 {
    const L0& l0;
    
    Level1(const L0& l0) : l0(l0) {
        std::cout << "Level1\t" << classname(typeid(this)) << "\t\t\t\t" << this << std::endl;
        std::cout << ">>> L0\t" << classname(typeid(&l0)) << "\t\t\t\t" << &l0 << std::endl << std::endl;
    }
    
    double operator()(const Point& p) const {
        return l0(p);
    }
};

template<typename L1>
struct Level2Base {
    const L1& l1;
    
    Level2Base(const L1& l1) : l1(l1) {
        std::cout << "Level2Base\t" << classname(typeid(this)) << "\t" << this << std::endl;
        std::cout << ">>> L1\t" << classname(typeid(&l1)) << "\t\t\t" << &l1 << std::endl;
        std::cout << ">>> >>> L0\t" << classname(typeid(&(l1.l0))) << "\t\t\t" << &(l1.l0) << std::endl << std::endl;
    }
    
    virtual double operator()(const Point& p) const = 0;
};

template<typename L1>
struct Level2Derived : Level2Base<L1> {
    Level2Derived(const L1& l1) : Level2Base<L1>(l1) {
        std::cout << "Level2Derived\t" << classname(typeid(this)) << "\t" << this << std::endl;
        std::cout << ">>> L1\t" << classname(typeid(&l1)) << "\t\t\t" << &l1 << std::endl;
        std::cout << ">>> >>> L0\t" << classname(typeid(&(l1.l0))) << "\t\t\t" << &(l1.l0) << std::endl << std::endl;
    }
    
    double operator()(const Point& p) const {
        return this->l1(p);
    }
};

int main() {
    Point p {2.0, 3.0};
    Level0 l0;
    Level1<decltype(l0)> l1(l0);
    Level2Derived<decltype(l1)> l2(l1);
    
    std::cout << l2(p) << std::endl;
    
    return 0;
}