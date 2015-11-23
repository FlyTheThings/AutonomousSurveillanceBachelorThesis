﻿#include "Configuration.h"
#include <iostream>

#define PI 3.14159265358979323846

namespace App
{

	Configuration::Configuration()
	{
		aStarCellSize = 20;
		mapNumber = 5;
		uavCount = 4;
		worldHeight = 1000;
		worldWidth = 1000;
		uavSize = 0.5;
		samplingRadius = 60;
		drawPeriod = 1;
		inputSamplesDist = 1;
		inputSamplesPhi = 3;
		rrtMinNodes = 1;
		rrtMaxNodes = 20000;
		nearCount = 1000;
		debug = true;
//		distanceOfNewNodes = 30;
//		guidingNearDist = 40;
		distanceOfNewNodes = 60;
		guidingNearDist = 80;
		numberOfSolutions = 10000;
		guidedSamplingPropability = 1;
		nearestNeighborMethod = NNMethod::Total;
		maxTurn = PI / 150;
		timeStep = 0.05;
		endTime = 0.5;
		relativeDistanceMax = 80;
		relativeDistanceMin = 10;
		localizationAngle = PI / 2;
		requiredNeighbors = 1;
		checkFov = false;
		allowSwarmSplitting = true;
		stop = false;
		textOutputEnabled = true;
		narrowPassageDivisor = 2;
		exitNarrowPassageTreshold = 5;
		narrowPassageCount = 0;
		divisionCount = 0;
	}

	int Configuration::getAStarCellSize() const
	{
		return aStarCellSize;
	}

	int Configuration::getMapNumber() const
	{
		return mapNumber;
	}

	int Configuration::getUavCount() const
	{
		return uavCount;
	}

	void Configuration::setMapNumber(int mapNumber)
	{
		this->mapNumber = mapNumber;
		if (core)	//kontrola prázdného pointeru
		{
			core->logConfigurationChange();
		}
	}

	void Configuration::setUavCount(int uavCount)
	{
		this->uavCount = uavCount;
	}

	void Configuration::setCore(shared_ptr<Core> core)
	{
		this->core = core;
	}

	int Configuration::getWorldHeight() const
	{
		return worldHeight;
	}

	void Configuration::setWorldHeight(int worldHeight)
	{
		this->worldHeight = worldHeight;
	}

	int Configuration::getWorldWidth() const
	{
		return worldWidth;
	}

	void Configuration::setWorldWidth(int worldWidth)
	{
		this->worldWidth = worldWidth;
	}

	double Configuration::getUavSize() const
	{
		return uavSize;
	}

	void Configuration::setUavSize(double uav_size)
	{
		uavSize = uav_size;
	}

	double Configuration::getSamplingRadius() const
	{
		return samplingRadius;
	}

	int Configuration::getDrawPeriod() const
	{
		return drawPeriod;
	}

	int Configuration::getInputSamplesDist() const
	{
		return inputSamplesDist;
	}

	int Configuration::getInputSamplesPhi() const
	{
		return inputSamplesPhi;
	}

	int Configuration::getInputCount() const
	{
		return pow(getInputSamplesDist() * getInputSamplesPhi(), getUavCount());
	}

	int Configuration::getRrtMinNodes() const
	{
		return rrtMinNodes;
	}

	int Configuration::getRrtMaxNodes() const
	{
		return rrtMaxNodes;
	}

	int Configuration::getNearCount() const
	{
		return nearCount;
	}

	bool Configuration::getDebug() const
	{
		return debug;
	}

	double Configuration::getDistanceOfNewNodes() const
	{
		return distanceOfNewNodes;
	}

	double Configuration::getGuidingNearDist() const
	{
		return guidingNearDist;
	}

	int Configuration::getNumberOfSolutions() const
	{
		return numberOfSolutions;
	}

	double Configuration::getGuidedSamplingPropability() const
	{
		return guidedSamplingPropability;
	}

	NNMethod Configuration::getNearestNeighborMethod() const
	{
		return nearestNeighborMethod;
	}

	double Configuration::getMaxTurn() const
	{
		return maxTurn;
	}

	double Configuration::getTimeStep() const
	{
		return timeStep;
	}

	double Configuration::getEndTime() const
	{
		return endTime;
	}

	double Configuration::getRelativeDistanceMin() const
	{
		return relativeDistanceMin;
	}

	double Configuration::getRelativeDistanceMax() const
	{
		return relativeDistanceMax;
	}

	double Configuration::getLocalizationAngle() const
	{
		return localizationAngle;
	}

	int Configuration::getRequiredNeighbors() const
	{
		return requiredNeighbors;
	}

	bool Configuration::getAllowSwarmSplitting() const
	{
		return allowSwarmSplitting;
	}

	bool Configuration::getCheckFov() const
	{
		return checkFov;
	}

	bool Configuration::getStop() const
	{
		return stop;
	}

	void Configuration::setStop(bool stop)
	{
		this->stop = stop;
	}

	bool Configuration::isTextOutputEnabled() const
	{
		return textOutputEnabled;
	}

	void Configuration::inNarrowPassage()
	{
		distanceOfNewNodes /= narrowPassageDivisor;
		maxTurn *= narrowPassageDivisor;
		guidingNearDist /= narrowPassageDivisor;
		narrowPassageCount = 0;
		divisionCount++;
	}

	void Configuration::outsideNarrowPassage()
	{
		narrowPassageCount++;
		//Far from obstacle
		if (narrowPassageCount > exitNarrowPassageTreshold)	//usv jsou daleko od překážek, navrácení původních hodnot
		{
			distanceOfNewNodes *= pow(narrowPassageDivisor, divisionCount);
			maxTurn /= pow(narrowPassageDivisor, divisionCount);
			guidingNearDist *= pow(narrowPassageDivisor, divisionCount);
			narrowPassageCount = 0;
			divisionCount = 0;
		}
	}
}
