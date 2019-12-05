
// DEMONSTRATE HOW TO XYPLOT IN FLTK
#include <Windows.h>
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Double_Window.H>
#include <string>	
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <memory>

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
	double maxX()
	{
		maximum.first = points[0].getX();
		int index = 1;
		while (index < points.size())
		{
			if (maximum.first < points[index].getX())
				maximum.first = points[index].getX();
			index++;
		}
		return maximum.first;
	}
	double minX()
	{
		minimum.first = points[0].getX();
		int index = 1;
		while (index < points.size())
		{
			if (minimum.first > points[index].getX())
				minimum.first = points[index].getX();
			index++;
		}
		return minimum.first;
	}
	double maxY()
	{
		maximum.second = points[0].getY();
		int index = 1;
		while (index < points.size())
		{
			if (maximum.second < points[index].getY())
				maximum.second = points[index].getY();
			index++;
		}
		return maximum.second;
	}
	double minY()
	{
		minimum.first = points[0].getY();
		int index = 1;
		while (index < points.size())
		{
			if (minimum.first > points[index].getY())
				minimum.first = points[index].getY();
			index++;
		}
		return minimum.first;
	}
};

class Graph : public Fl_Widget
{
	Points points;
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
	void bounds()
	{
		minmaxX.first = points.minX();
		minmaxX.second = points.maxX();
		minmaxY.first = points.minY();
		minmaxY.second = points.maxY();
		axisrange.first = minmaxX.second - minmaxX.first;
		axisrange.second = minmaxY.second - minmaxY.first;
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
		for (double i = interval; i <= w() + interval; i += interval)
		{
			Point p = scalePointX(index++, i);
			xtransform.set(p);
		}
	}
	Point scalePointY(Point point)
	{
		Point py(point.getX(), (1.0 - (((point.getY() - minmaxY.first))) / axisrange.second) * h());
		return py;
	}
	void scaleY()
	{
		for (int i = 0; i < points.size(); i++)
		{
			Point p = scalePointY(points[i]);
			ytransform.set(p.getX(), p.getY());
		}
	}
	void draw()
	{
		fl_color(FL_BLACK);
		fl_line_style(FL_SOLID, 3, NULL);
		for (int i = 1; i < size(); i++)
		{
			Point p1 = xtransform[i - 1];
			Point p2 = xtransform[i];
			fl_line(p1.getX(), p1.getY(), p2.getX(), p2.getY());
		}
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
	void set(Points& p)
	{
		for (int i = 0; i < p.size(); i++)
		{
			Point pt = p[i];
			graph->add(pt);
		}
		graph->bounds();
		graph->scaleY();
		graph->scaleX();
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
	vector<string> ParseString(string toParse, char delimiter)
	{
		vector<string> vs;
		int start = 0;
		int position = -1;
		while ((position = toParse.find(delimiter, start)) != -1)
		{
			string s = toParse.substr(start, position - start);
			vs.push_back(s);
			start = position + 1;
		}
		if (start > 0)
			vs.push_back(toParse.substr(start, toParse.length() - start));
		return vs;
	}
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
				while (!in.eof())
				{
					getline(in, line);
					vector<string> vs = ParseString(line, ',');
					if (vs.size() > 0)
					{
						double n1 = stod(vs[0], NULL);
						double n2 = stod(vs[1], NULL);
						points.set(n1, n2);
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
	Points regLine;
public:
	Regression()
	{
		sumX = sumX2 = sumY = sumXY = 0;
	}
	pair<double, double> calc(Points& ps)
	{
		int n = ps.size();
		for (int i = 0; i < n; i++)
		{
			sumX += ps[i].getX();
			sumX2 += ps[i].getX() * ps[i].getX();
			sumY += ps[i].getY();
			sumXY += ps[i].getX() * ps[i].getY();
		}
		double k = (n * sumXY - sumX * sumY) / (n * sumX2 - sumX * sumX);
		double b = (sumY - k * sumX) / n;
		double start = ps.minX() - 0.5;
		regLine.set(start, start * k + b);
		regLine.set(start+1, (start + 1) * k + b);
		regLine.set(start+2, (start + 2) * k + b);
		regLine.set(start+3, (start + 3) * k + b);
		regLine.set(start+4, (start + 4) * k + b);
		regLine.set(start + 5, (start + 5) * k + b);
		regLine.set(start + 6, (start + 6) * k + b);
		regLine.set(start + 7, (start + 7) * k + b);
		regLine.set(start + 8, (start + 8) * k + b);
		regLine.set(start + 9, (start + 9) * k + b);
		regLine.set(start + 10, (start + 10) * k + b);
		regLine.set(start + 11, (start + 11) * k + b);
		regLine.set(start + 12, (start + 12) * k + b);
		return make_pair(k, b);
	}
	Points getRegLine()
	{
		return regLine;
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
		cout << "Regression Line: y=" << regLine.first << "x+" << regLine.second;
		Points line = reg.getRegLine();
		int width = 800;
		int height = 600;
		string caption = "XY Plot";
		XYPlot plot(0, 0, width, height, caption);
		plot.start();
		plot.set(line);
		plot.draw();
	}
};

void main()
{
	//int width = 800;
	//int height = 600;
	//string caption = "XY Plot";
	//Points points = SetPoints("scatter.csv");
	//XYPlot plot(0, 0, width, height, caption);
	//plot.start();
	//plot.set(points);
	//plot.draw();
	DrawRegression drawing("scatter.csv");
	drawing.disPlay();
}