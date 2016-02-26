#include "Resampler.h"


Resampler::Resampler(shared_ptr<Configuration> configuration) : configuration(configuration)
{
}

vector<shared_ptr<State>> Resampler::resampleToMaxFrequency(vector<shared_ptr<State>> path)
{
	double timeStep = configuration->getTimeStep();
	int sampleCount = path.size();
	double totalTime = timeStep * sampleCount;	//total time to fly the path
	double currentFrequency = sampleCount / totalTime;	//samples per second
	double maxFrequency = configuration->getMaxSampleFrequency();
	double maxSampleCount = configuration->getMaxSampleCount();
	int maxAvailableFrequency = maxSampleCount / totalTime;
	double newFrequency = min<double>(maxAvailableFrequency, maxFrequency);	//pokud je maxFrequency v�t�� ne� maxAvailableFrequency, bude nastavena maxAvailable, jinak maaxFrequency
	double ratio = newFrequency / currentFrequency;	//bude ratio kr�t v�ce vzork�
	double newTimeStep = 1 / newFrequency;

	vector<shared_ptr<State>> newPath = vector<shared_ptr<State>>();
	newPath.push_back(make_shared<State>(*path[0]));
	for (size_t currentTime = newTimeStep; currentTime < totalTime; currentTime += newTimeStep)	//prom�nnou currentTime pojedu od 0 do konce a o�ek�v�m, �e p�evzorkov�n�m trajektorii t�m�� nezm�n�m, UAV bude ve stejn� �as na stejn�m m�st�
	{

	}
}

Resampler::~Resampler()
{
}
