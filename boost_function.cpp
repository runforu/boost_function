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
        return new func(*this);
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
        return new member(*this);
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
        return new functor(*this);
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

class Point {
public:
    int get(int a) {
        std::cout << "Point::get called: " << std::endl;
        return 2;
    }
    int doit(Point* a, int b) {
        std::cout << "Point::doit called: " << std::endl;
        return 1;
    }
};

int get(Point* p, int b) {
    std::cout << "function pointer." << std::endl;
    return 0;
}

int main(int argc, char const* argv[]) {
    std::vector<function<int(Point*, int)>> vf;

    Point point;
    function<int(Point*, int)> f(get);
    vf.push_back(f);

    function<int(Point*, int)> m(&Point::get);
    vf.push_back(m);

    function<int(Point*, int)> obj(boost::bind(&Point::doit, &point, _1, _2));
    vf.push_back(obj);

    for (std::vector<function<int(Point*, int)>>::iterator it = vf.begin(); it != vf.end(); it++) {
        (*it)(&point, 33);
    }
}