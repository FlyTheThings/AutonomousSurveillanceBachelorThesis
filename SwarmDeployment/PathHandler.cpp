#include "PathHandler.h"

namespace App
{

	PathHandler::PathHandler()
	{
	}


	PathHandler::~PathHandler()
	{
	}

	vector<shared_ptr<LinkedState>> PathHandler::getPath(shared_ptr<LinkedState> end)
	{
		vector<shared_ptr<LinkedState>> path = vector<shared_ptr<LinkedState>>();
		auto iterNode = end;
		do
		{
			path.push_back(iterNode);
			iterNode = iterNode->getPrevious();
		} while (iterNode->getPrevious());

		path.push_back(iterNode);

		reverse(path.begin(), path.end());	//abych m�l cestu od za��tku do konce
		return path;
	}

	vector<shared_ptr<State>> PathHandler::createStatePath(vector<shared_ptr<LinkedState>> path)
	{
		vector<shared_ptr<State>> newPath = vector<shared_ptr<State>>(path.size());
		for (size_t i = 0; i < path.size(); i++)
		{
			newPath[i] = make_shared<State>(*path[i].get());
		}

		return newPath;
	}

	vector<shared_ptr<LinkedState>> PathHandler::getPath(shared_ptr<LinkedState> start, shared_ptr<LinkedState> end)
	{
		vector<shared_ptr<LinkedState>> path = vector<shared_ptr<LinkedState>>();
		auto iterNode = end;
		do
		{
			path.push_back(iterNode);
			iterNode = iterNode->getPrevious();
		} while (*iterNode.get() != *start.get());

		path.push_back(iterNode);

		reverse(path.begin(), path.end());	//abych m�l cestu od za��tku do konce
		return path;
	}

}