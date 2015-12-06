#include "GuiDrawer.h"
#include <QGraphicsLineItem>
#include <QtWidgets/QMainWindow>
#include <iostream>
#include <QtCore/qtextstream.h>
#include <QtWidgets/QMessageBox>
#include "mainwindow.h"
#include "Configuration.h"

namespace Ui
{

	GuiDrawer::GuiDrawer(QGraphicsView* view, QMainWindow* window, ::MainWindow* mainWindow, QTextBrowser* text) :
		view(view),
		scene(view->scene()),
		window(window),
		mainWindow(mainWindow),
		text(text)
	{
		LoggerInterface();
	}

	GuiDrawer::~GuiDrawer()
	{
	}

	void GuiDrawer::logSelectedMap(shared_ptr<Map> map, int worldWidth, int worldHeight)
	{
		QTextStream cout(stdout);
		cout << "logging selected map" << endl;
		clear();

		view->resize(worldWidth, worldHeight);

		for (auto goal : map->getGoals())
		{
			auto r = goal->getRectangle();
			scene->addRect(r->getX(), r->getY(), r->getWidth(), r->getHeight(), QPen(Qt::green), QBrush(Qt::green));
		}

		for (auto obstacle : map->getObstacles())
		{
			auto r = obstacle->rectangle;
			scene->addRect(r->getX(), r->getY(), r->getWidth(), r->getHeight(), QPen(Qt::gray), QBrush(Qt::gray));
		}

		for (auto start : map->getUavsStart())
		{
			auto p = start->getPointParticle()->getLocation();
			Qt::GlobalColor uavColor = getRandomColor();
			uavColors[*start.get()] = uavColor;
			addCross(p->getX(), p->getY(), 3, uavColor);
		}

		drawGrid();
		mainWindow->updateView();
	}

	void GuiDrawer::logMapGrid(vector<vector<Grid>> mapGrid)
	{
//		int cellSize = configuration->getAStarCellSize();
//		int x = 5;
//		for (auto row : mapGrid)
//		{
//			int y = 35;
//			for (auto grid : row)
//			{
//				std::string gridText;
//				switch (grid)
//				{
//				case App::Grid::Obstacle: gridText = "obstacle"; break;
//				case App::Grid::Free: gridText = "free"; break;
//				case App::Grid::Goal: gridText = "goal"; break;
//				case App::Grid::UAV: gridText = "uav"; break;
//				}
//				addText(QString::fromStdString(gridText), x, y);
//				y += cellSize;
//			}
//			x += cellSize;
//		}
//		mainWindow->updateView();
	}

	void GuiDrawer::logMapNodes(vector<shared_ptr<Node>> nodes)
	{
		int cellSize = configuration->getAStarCellSize();
		int x = - 12;
		int y = 12;
		for (auto node : nodes)
		{
			addText(QString::fromStdString(to_string(int(node->getCost()))), node->getPoint()->getX() + x, node->getPoint()->getY() + y);
//			addText(QString::fromStdString(to_string(int(node->getDistanceToObstacle()))), node->getPoint()->getX() + x, node->getPoint()->getY() + y);
		}
		mainWindow->updateView();
	}

	void GuiDrawer::logAStarNode(shared_ptr<AStar::NodeWrapper> node)
	{
//		auto previous = node->getParent();
//		scene->addLine(previous->getX(), previous->getY(), node->getX(), node->getY(), QPen(Qt::green));
//		mainWindow->updateView();
	}

	void GuiDrawer::logGuidingPaths(vector<shared_ptr<Path>> paths, shared_ptr<Node> start, vector<tuple<shared_ptr<Node>, shared_ptr<GoalInterface>>> ends)
	{
		int cellSize = configuration->getAStarCellSize();
//		scene->addRect(start->getPoint()->getX() - cellSize/2, start->getPoint()->getY() - cellSize/2, cellSize, cellSize, QPen(Qt::darkCyan), QBrush(Qt::darkCyan));
//		for (auto end : ends)
//		{
//			auto endPoint = get<0>(end);
//			scene->addRect(endPoint->getPoint()->getX() - cellSize/2, endPoint->getPoint()->getY() - cellSize/2, cellSize, cellSize, QPen(Qt::yellow), QBrush(Qt::yellow));
//		}

		for (auto path : paths)
		{
			shared_ptr<Node> previous = nullptr;
			for (auto node : path->getNodes())
			{
				if (previous != nullptr)
				{
					auto line = scene->addLine(previous->getPoint()->getX(), previous->getPoint()->getY(), node->getPoint()->getX(), node->getPoint()->getY(), QPen(Qt::blue));
				}
				previous = node;
			}
		}
		mainWindow->updateView();
	}

	void GuiDrawer::logText(string string)
	{
		if (configuration->isTextOutputEnabled())
		{
			//		showPopup(string);
			text->append(QString::fromStdString(string));
			//		clear();
			//		addText(QString::fromStdString(string), 20, 20);
			mainWindow->updateView();
			//		view->repaint();
		}
	}

	void GuiDrawer::logText(char const arr[])
	{
		logText(string(arr));
	}

	void GuiDrawer::logNewState(shared_ptr<State> nearNode, shared_ptr<State> newNode)
	{
		for (size_t i = 0; i < nearNode->uavs.size(); i++)
		{
			auto uav = nearNode->uavs[i];
			scene->addLine(nearNode->uavs[i]->getPointParticle()->getLocation()->getX(), nearNode->uavs[i]->getPointParticle()->getLocation()->getY(),
				newNode->uavs[i]->getPointParticle()->getLocation()->getX(), newNode->uavs[i]->getPointParticle()->getLocation()->getY(), QPen(uavColors[*uav.get()]));
		}
		mainWindow->updateView();
	}

	void GuiDrawer::logRandomStates(unordered_map<Uav, shared_ptr<Point>, UavHasher> randomStates)
	{
//		for (auto pair : randomStates)
//		{
//			auto uav = pair.first;
//			auto point = pair.second;
//			scene->addEllipse(point->getX(), point->getY(), 2, 2, QPen(uavColors[uav]));	//todo? zjistit, jak se iteruje p�es unordered_map
////			addCross(randomStates[i]->getX(), randomStates[i]->getY(), 3, uavColors[i]);
//		}
//		mainWindow->updateView();
	}

	void GuiDrawer::logRandomStatesCenter(shared_ptr<Point> center)
	{
		scene->addEllipse(center->getX(), center->getY(), 3, 3, QPen(Qt::black), QBrush(Qt::black));
		mainWindow->updateView();
	}

	void GuiDrawer::setConfiguration(shared_ptr<Configuration> configuration)
	{
		this->configuration = configuration;
	}

	void GuiDrawer::logBestPath(vector<shared_ptr<State>> path)
	{
		bool first = true;
		for (auto state : path)
		{
			//p�eskakuji prvn� iteraci, proto�e kresl�m cesty mezi st�vaj�c�m a p�edchoz�m stavem
			if (first)
			{
				first = false;
				continue;
			}

			auto previous = state->getPrevious();
			for (size_t i = 0; i < state->uavs.size(); i++)
			{
				auto uav = state->uavs[i];
				scene->addLine(state->uavs[i]->getPointParticle()->getLocation()->getX(), state->uavs[i]->getPointParticle()->getLocation()->getY(),
					previous->uavs[i]->getPointParticle()->getLocation()->getX(), previous->uavs[i]->getPointParticle()->getLocation()->getY(), QPen(uavColors[*uav.get()], 4));
			}
			mainWindow->updateView();

		}
	}

	void GuiDrawer::clear()
	{
		double height = view->height();
		double width = view->width();

		scene->addRect(0, 0, width, height, QPen(Qt::white), QBrush(Qt::white));
	}

	void GuiDrawer::drawGrid()
	{
		double height = view->height();
		double width = view->width();

		int cellSize = configuration->getAStarCellSize();
		for (int i = 0; i <= height; i += cellSize)
		{
			scene->addLine(i, 0, i, height, QPen(Qt::gray));
			QString number = QString("%1").arg(i);
			addText(number, i - 0 - 6 * number.size(), 0);
		}

		for (int i = 0; i <= width; i += cellSize)
		{
			scene->addLine(0, i, width, i, QPen(Qt::gray));
			QString number = QString("%1").arg(i);
			addText(number, 0 - 10 - 7 * number.size(), i + 12);
		}

	}

	QGraphicsTextItem* GuiDrawer::addText(QString text, double x, double y)
	{
		auto textItem = scene->addText(text);
		textItem->moveBy(x, y);
		textItem->setFlag(QGraphicsItem::ItemIgnoresTransformations);
		return textItem;
	}

	void GuiDrawer::addCross(double x, double y, double size, Qt::GlobalColor color)
	{
		scene->addLine(x - size, y - size, x + size, y + size, QPen(color));
		scene->addLine(x - size, y + size, x + size, y - size, QPen(color));
	}

	Qt::GlobalColor GuiDrawer::getRandomColor()
	{
		Qt::GlobalColor blackList[] = { Qt::white, Qt::transparent };
		Qt::GlobalColor color = Qt::GlobalColor(rand() % Qt::transparent);	//tansparent is last color of enum
		for (auto blacklisted : blackList)
		{
			if (color == blacklisted)
			{
				color = Qt::GlobalColor(rand() % Qt::transparent);
			}
		}
		for (auto pair : uavColors)
		{
			auto another = pair.second;
			if (color == another)
			{
				color = Qt::GlobalColor(rand() % Qt::transparent);
			}

		}
		return color;
	}

	void GuiDrawer::showPopup(string text)
	{
		QMessageBox::information(
			window,
			"Logged text",
			QString::fromStdString(text));
	}
}