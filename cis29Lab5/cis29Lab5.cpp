
// DEMONSTRATE HOW TO XYPLOT IN FLTK
#include <Windows.h>
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Double_Window.H>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <memory>
#include <functional>
#include <regex>

using namespace std;

class Point : pair<double,double>
{
	double X = 0;
	double Y = 0;
public:
	Point(double a, double b)
	{
		first = a;
		second = b;
		X = first;
		Y = second;
	}
	double getX() { return X; }
	double getY() { return Y; }
};

class Points
{
	vector<Point> points;
	pair<double, double> minimum;
	pair<double, double> maximum;
	function<bool(Point, Point)> compareX = [](Point l, Point r) { return l.getX() < r.getX(); };
	function<bool(Point, Point)> compareY = [](Point l, Point r) { return l.getY() < r.getY(); };
public:
	int size() { return points.size(); }
	Point get(int index) { return points[index]; }
	Point operator [] (int index) { return get(index); }
	void set(Point p) { points.push_back(p); }
	void set(double x, double y)
	{
		Point p(x, y);
		points.push_back(p);
	}
	pair<double, double> minmaxX()
	{
		auto iminmax = minmax_element(points.begin(), points.end(), compareX);
		return make_pair(iminmax.first->getX(), iminmax.second->getX());
	}
	pair<double, double> minmaxY()
	{
		auto iminmax = minmax_element(points.begin(), points.end(), compareY);
		return make_pair(iminmax.first->getY(), iminmax.second->getY());
	}
	void for_all(function<void(Point)> fun)
	{
		const auto _ = for_each(points.begin(), points.end(), fun);
	}
	double get_sum(function<double(Point)> fun)
	{
		return accumulate(points.begin(), points.end(), 0.0, [&](double sum, Point i) {return sum + fun(i); });
	}
};

class Graph : public Fl_Widget
{
	Points points;
	pair<double, double> line;
	string lineFunction;
	Points lineTransformed;
	Points xtransform;
	Points ytransform;
	pair<double, double> axisrange;
	pair<double, double> minmaxX;
	pair<double, double> minmaxY;
	double border = 10;
public:
	Graph(int X, int Y, int W, int H, string S = "") : Fl_Widget(X, Y, W, H, S.data()) { }
	int size() { return points.size(); }
	void add(double x, double y) { points.set(x, y); }
	void add(Point& p) { points.set(p); }
	void setLine(double k, double b) { line = make_pair(k, b); }
	void setLine(pair<double, double> p) { line = p; }
	void setLineFunction(string f) { lineFunction = f; }
	void bounds()
	{
		minmaxX = points.minmaxX();
		minmaxY = points.minmaxY();
		axisrange = make_pair(minmaxX.second - minmaxX.first, minmaxY.second - minmaxY.first);
	}
	Point scalePointX(int index, double interval)
	{
		Point px(interval, ytransform[index].getY());
		return px;
	}
	void scaleX() // assumes evenly spaced X intervals
	{
		int index = 0;
		double interval = floor((double)w() / (points.size()));
		points.for_all([&](Point i) {xtransform.set(scalePointX(index++, index*interval)); });
	}
	Point scalePointY(Point point)
	{
		Point py(point.getX(), (1.0 - (((point.getY() - minmaxY.first))) / axisrange.second) * h());
		return py;
	}
	void scaleY()
	{
		points.for_all([&](Point i) {
			Point p = scalePointY(i); 
			ytransform.set(p.getX(), p.getY()); });
	}
	void scaleLine()
	{
		function<double(double)> lineFunc = [=](double x) {return x * line.first + line.second; };
		Point lineBeg(points.minmaxX().first, lineFunc(points.minmaxX().first));
		Point lineEnd(points.minmaxX().second, lineFunc(points.minmaxX().second));
		lineTransformed.set(xtransform.minmaxX().first, scalePointY(lineBeg).getY());
		lineTransformed.set(xtransform.minmaxX().second, scalePointY(lineEnd).getY());
	}
	void draw()
	{
		fl_color(FL_BLACK);
		fl_line_style(FL_SOLID, 3, NULL);
		xtransform.for_all([](Point i) { fl_circle(i.getX(), i.getY(), 3.5); });
		fl_color(FL_RED);
		fl_line_style(FL_SOLID, 3, NULL);
		fl_draw(lineFunction.data(), lineTransformed[0].getX(), lineTransformed[0].getY());
		fl_line(lineTransformed[0].getX(), lineTransformed[0].getY(), lineTransformed[1].getX(), lineTransformed[1].getY());
		
	}
};

class XYPlot
{
	unique_ptr<Fl_Double_Window> window;
	unique_ptr<Graph> graph;
	pair<int, int> origin;
	pair<int, int> dimension;
	string caption;
public:
	XYPlot(int x, int y, int w, int h, string s)
	{
		origin.first = x;
		origin.second = y;
		dimension.first = w;
		dimension.second = h;
		caption = s;
	}
	void start()
	{
		window = make_unique<Fl_Double_Window>(dimension.first, dimension.second, caption.data());
		graph = make_unique<Graph>(origin.first, origin.second, dimension.first, dimension.second);
		window->resizable(graph.get());
	}
	void set(Points& p, pair<double, double>& l, string& strFun)
	{
		p.for_all([&](Point i) {graph->add(i);});
		graph->setLine(l);
		graph->setLineFunction(strFun);
		graph->bounds();
		graph->scaleY();
		graph->scaleX();
		graph->scaleLine();
	}
	int draw()
	{
		window->show();
		return(Fl::run());
	}
};

class FilePraser
{
private:
	Points points;
	string filename;
public:
	FilePraser(string i)
	{
		filename = i;
	}
	void SetPoints()
	{
		try
		{
			ifstream in(filename);
			if (in.is_open())
			{
				string line;
				smatch match;
				regex pattern("(.*),(.*)");
				while (!in.eof())
				{
					getline(in, line);
					if (regex_search(line, match, pattern))
					{
						points.set(stod(match[1], NULL), stod(match[2], NULL));
					}
				}
				in.close();
			}
			else
			{
				cout << "Unable to open file " << filename << endl;
				exit(0);
			}
		}
		catch (ifstream::failure e)
		{
			cout << e.what() << endl;
			exit(0);
		}
	}
	Points getPoints()
	{
		return points;
	}
};

class Regression
{
private:
	double sumX;
	double sumX2;
	double sumY;
	double sumXY;
	pair<double, double> slope_intercept;
public:
	Regression()
	{
		sumX = sumX2 = sumY = sumXY = 0;
	}
	pair<double, double> calc(Points& ps)
	{
		int n = ps.size();
		sumX = ps.get_sum([](Point i) {return i.getX(); });
		sumY = ps.get_sum([](Point i) {return i.getY(); });
		sumX2 = ps.get_sum([](Point i) {return i.getX()* i.getX(); });
		sumXY = ps.get_sum([](Point i) {return i.getX() * i.getY(); });
		double k = (n * sumXY - sumX * sumY) / (n * sumX2 - sumX * sumX);
		double b = (sumY - k * sumX) / n;
		double start = ps.minmaxX().first;
		double end = ps.minmaxY().second;
		slope_intercept = make_pair(k, b);
		return slope_intercept;
	}
	string getLineFunction()
	{
		return "Regression Line: y=" + to_string(slope_intercept.first) + "x+" + to_string(slope_intercept.second);
	}
};

class DrawRegression
{
private:
	Points points;
	string inputFilename;
public:
	DrawRegression(string i)
	{
		FilePraser fp(i);
		fp.SetPoints();
		points = fp.getPoints();
	}
	void disPlay()
	{
		Regression reg;
		pair<double, double> regLine = reg.calc(points);
		string strFun = reg.getLineFunction();
		int width = 800;
		int height = 600;
		string caption = "XY Plot";
		XYPlot plot(0, 0, width, height, caption);
		plot.start();
		plot.set(points, regLine, strFun);
		plot.draw();
	}
};

void main()
{
	DrawRegression drawing("scatter.csv");
	drawing.disPlay();
}