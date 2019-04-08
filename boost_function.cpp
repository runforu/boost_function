#include <boost/bind.hpp>
#include <iostream>
#include <vector>

template <typename R, typename T1, typename T2>
class base {
public:
    virtual ~base() {}

    virtual R operator()(T1, T2) = 0;

    virtual base* clone() = 0;
};

template <typename R, typename T1, typename T2>
class func : public base<R, T1, T2> {
public:
    func(R (*ptr)(T1, T2)) : ptr_(ptr) {}

    virtual R operator()(T1 a, T2 b) {
        return ptr_(a, b);
    }

    virtual func* clone() {
        return new func(ptr_);
    }

private:
    R (*ptr_)(T1, T2);
};

template <typename R, typename Class, typename T>
class member : public base<R, Class, T> {};

template <typename R, typename Class, typename T>
class member<R, Class*, T> : public base<R, Class*, T> {
public:
    member(R (Class::*ptr)(T)) : ptr_(ptr) {}

    virtual R operator()(Class* obj, T a) {
        return (obj->*ptr_)(a);
    }

    virtual member* clone() {
        return new member(ptr_);
    }

private:
    R (Class::*ptr_)(T);
};

template <typename T, typename R, typename T1, typename T2>
class functor : public base<R, T1, T2> {
public:
    functor(const T& obj) : obj_(obj) {}

    virtual R operator()(T1 a, T2 b) {
        return obj_(a, b);
    }

    virtual functor* clone() {
        return new functor(obj_);
    }

private:
    T obj_;
};

template <typename T>
class function {};

template <typename R, typename T1, typename T2>
class function<R(T1, T2)> {
public:
    template <typename Class, typename _R, typename _T2>
    function(_R (Class::*ptr)(_T2)) : ptr_(new member<R, T1, T2>(ptr)) {}

    template <typename _R, typename _T1, typename _T2>
    function(_R (*ptr)(_T1, _T2)) : ptr_(new func<R, T1, T2>(ptr)) {}

    template <typename T>
    function(const T& obj) : ptr_(new functor<T, R, T1, T2>(obj)) {}

    ~function() {
        delete ptr_;
    }

    virtual R operator()(T1 a, T2 b) {
        return ptr_->operator()(a, b);
    }

    function(const function& f) {
        ptr_ = f.ptr_->clone();
    }

private:
    base<R, T1, T2>* ptr_;
};

class Base {
    virtual int VirtualMethod(int b) = 0;
};

class Point : public Base {
public:
    int NormalMethod(int a) {
        std::cout << "Point::NormalMethod called: " << std::endl;
        return 1;
    }
    int BindMethod(Point* a, int b) {
        std::cout << "Point::BindMethod called: " << std::endl;
        return 1;
    }
    static int StaticMethod(Point* a, int b) {
        std::cout << "static Point::StaticMethod called: " << std::endl;
        return 1;
    }
    virtual int VirtualMethod(int b) {
        std::cout << "virtual Point::VirtualMethod called: " << std::endl;
        return 1;
    }
};

int NormalFunction(Point* p, int b) {
    std::cout << "NormalFunction pointer." << std::endl;
    return 0;
}

int main(int argc, char const* argv[]) {
    std::vector<function<int(Point*, int)>> fvector;

    Point point;

    function<int(Point*, int)> nf(NormalFunction);
    fvector.push_back(nf);

    function<int(Point*, int)> nm(&Point::NormalMethod);
    fvector.push_back(nm);

    function<int(Point*, int)> bind(boost::bind(&Point::BindMethod, &point, _1, _2));
    fvector.push_back(bind);

    function<int(Point*, int)> sm(&Point::StaticMethod);
    fvector.push_back(sm);

    function<int(Point*, int)> vm(&Point::VirtualMethod);
    fvector.push_back(vm);

    for (std::vector<function<int(Point*, int)>>::iterator it = fvector.begin(); it != fvector.end(); it++) {
        (*it)(&point, 33.0);
    }

    return 0;
}