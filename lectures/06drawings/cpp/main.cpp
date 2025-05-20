#define _CRT_SECURE_NO_WARNINGS

#include "canvas.h"
#include <string>
#include <memory>

#include <crtdbg.h>

struct shape { // abstract class
	int x_, y_;
	char c_;
	std::string name_;

	shape(int x, int y, char c) : x_(x), y_(y), c_(c), name_("shape") {
	}
	/*
	virtual ~shape() {

	}
	*/
	
	virtual ~shape() = default; // virtual otherwise if we allocate memory in classes that inherit it will never be freed
		// it has a cost (double indirection: first access the virtual table and then the function)
		// since we have to write virtual we can't just remove the constructor
	/*
	virtual void draw(canvas& c) const { // virtual tells the compiler we want a virtual table inside our objects
		// from now on the classes inheriting from this class will call the correct method 
		// the virtual table will tell the compiler the type of the child class 
		// if the children classes if we don't put const as in this method's signature it won't call it 
		// because they will be interpreted as different functions (same if we forget a reference of anything else)
		// we can solve it by using override (explicitly telling the compiler we are overriding a virtual function)
		// it tells us if we don't match the signature (it doesn't compile)
		c.set(x_, y_, c_); // dummy
	}
	*/
	virtual void draw(canvas& c) const = 0; // draw function doesn't exist (pure virtual function)
	
	void setname(std::string name) {
		name_ = std::move(name);
	}
};

struct point : public shape {
	//int* what_; // more resources
	std::unique_ptr<int> what_;
	point(int x, int y, char c) : shape(x, y, c) {
		//what_ = new int;
		what_ = std::make_unique<int>(0x00a0ccca);
		//what_[0] = 0x00a0ccca;
		setname("point");
	}
	/* destructor of unique_ptr will be called when we call delete on point object
	~point() {
		delete what_;
	}
	*/
	
	// since shape has no more a draw function point has to implement it 
	void draw(canvas& c) const {
		c.set(x_, y_, c_); // dummy
	}
	// we should implement copy constructor and assignment operator
};

struct line : public shape {
	int x1_, y1_;

	line(int x, int y, int x1, int y1, char c) : shape(x, y, c), x1_(x1), y1_(y1) {
		setname("line");
	}
	/*
	~line() {
	}
	*/
	
	void draw(canvas& c) const {
		c.line(x_, y_, x1_, y1_, c_);
	}
};

struct rectangle: public shape {
	int x1_, y1_;

	rectangle(int x, int y, int x1, int y1, char c) : shape(x, y, c), x1_(x1), y1_(y1) {
		setname("rectangle");
	}
	/*
	~rectangle() {
	}
	*/
	
	void draw(canvas& c) const override {
		c.rectangle(x_, y_, x1_, y1_, c_);
	}
};

struct circle : public shape {
	int r_;

	circle(int x, int y, int r, char c) : shape(x, y, c), r_(r) {
		setname("circle");
	}
	/*
	~circle() {
	}
	*/
	
	void draw(canvas& c) const {
		c.circle(x_, y_, r_, c_);
	}
};


int main(void)
{
	{
		canvas c(80, 25);

		/*
		auto r1 = std::make_unique<rectangle>(0, 0, 79, 24, '*');
		auto p1 = std::make_unique<point>(5, 15, '?');
		auto c1 = std::make_unique<circle>(10, 10, 4, '@');
		auto c2 = std::make_unique<circle>(70, 10, 4, '@');
		auto l1 = std::make_unique<line>(40, 15, 40, 20, '|');
		*/

		// we need to choose who is the owner of the memory, we can't copy unique ptrs, we have to move them
		//std::vector<unique_ptr<shape>> shapes = { std::move(r1), std::move(p1), std::move(c1), std::move(c2), std::move(l1) };

		std::vector<std::unique_ptr<shape>> shapes;
		/*
		shapes.push_back(std::move(r1));
		shapes.push_back(std::move(p1));
		shapes.push_back(std::move(c1));
		shapes.push_back(std::move(c2));
		shapes.push_back(std::move(l1));
		*/
		shapes.push_back(std::make_unique<rectangle>(0, 0, 79, 24, '*'));
		shapes.push_back(std::make_unique<point>(5, 15, '?'));
		shapes.push_back(std::make_unique<circle>(10, 10, 4, '@'));
		shapes.push_back(std::make_unique<circle>(70, 10, 4, '@'));
		shapes.push_back(std::make_unique<line>(40, 15, 40, 20, '|'));

		



		for (const auto& s : shapes) {
			s->draw(c);
		}

		/*
		rectangle* r1 = new rectangle(0, 0, 79, 24, '*');
		point* p1 = new point(5, 15, '?');
		circle* c1 = new circle(10, 10, 4, '@');
		circle* c2 = new circle(70, 10, 4, '@');
		line* l1 = new line(40, 15, 40, 20, '|');

		shape* shapes[] = { r1, p1, c1, c2, l1 };
		size_t nshapes = 5;

		for (size_t i = 0; i < nshapes; ++i) {
			shapes[i]->draw(c); 
		}
		*/
		
		c.out(stdout);

		/*
		for (size_t i = 0; i < nshapes; ++i) {
			delete shapes[i];
		}
		*/
		
	}
	_CrtDumpMemoryLeaks();
}
