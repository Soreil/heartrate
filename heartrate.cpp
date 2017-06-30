#include "main.h"
#include "heartrate.h"
//P0_5 for analog reading of SPO2 sensor
AnalogIn spo2(P0_5);

const unsigned int Ts = 5; //Sampling time
Timer t; //millis() source

unsigned long previousTime = t.read_ms(); //remembers the last millis

//effect: averaging filter on the inputs.
//Looks at times between upgoing flanks
//Calculates the flanks per minute
//if its between 30 and 230 and is not 15 away from your last flanks/minute
//print the average of the last 5 flanks per minutes
float calculateHeartRate(std::vector<unsigned long>& samples) {
	float currentTime;
	float oldTime;
	bool timehelper = 0;

	float result = 0;
	long sensorSum = 0;
	long oldSensorSum = 0;
	std::vector<float> outputcalculated;
	outputcalculated.reserve(peaksSampled+1);

	for (int n : samples) {
		sensorSum += n / samples.size();
	}

	if (sensorSum - oldSensorSum < 3 && timehelper)
	{
		currentTime = t.read_ms();
		outputcalculated.insert(outputcalculated.begin(), minute / ((currentTime - oldTime) / 1000000));
		if (outputcalculated[0] > lowerLimit && outputcalculated[0] < upperLimit) {
			int delta = outputcalculated[0] - outputcalculated[1];
			if (delta < upperMargin && delta > lowerMargin) {
				result = average(outputcalculated);
			}
			outputcalculated.pop_back();
		}
		oldTime = currentTime;
		timehelper = false;
	}
	else if ((sensorSum - oldSensorSum > 3) && !timehelper) {
		timehelper = true;
	}
	oldSensorSum = sensorSum;
	return result;
}

float average(std::vector<float> const& v) {
	return 1.0 * std::accumulate(v.begin(), v.end(), 0LL) / v.size();
}

//TODO(sjon): Here we will be reading from the ADC instead of this sample code.
void updateSensorValue() {
	hrServicePtr->updateHeartRate(hrmCounter);
}

void loop() {
	const int sampleCount = 40; //Samples seperated by Ts

	std::vector<unsigned long> samples; //vector of our measurements from the sensor
	samples.reserve(sampleCount);

	//if sampling time has elapsed read the sensor, calculate output and then shift x
	if (t.read_ms() - previousTime > Ts) {
		previousTime = t.read_ms();
		samples.insert(samples.begin(), spo2.read_u16());
		calculateHeartRate(samples);
		samples.pop_back();//drop final sample.
	}
}