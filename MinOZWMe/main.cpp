#include "../OpenZWaveMe/src/Manager.h"

using namespace OpenZWaveMe;

int main()
{
    Options::Create( "/usr/local/etc/openzwave/", "", "");

	Options::Get()->AddOptionInt("SaveLogLevel", OpenZWave::LogLevel_Detail);
	Options::Get()->AddOptionInt("QueueLogLevel", OpenZWave::LogLevel_Debug);
	Options::Get()->AddOptionInt("DumpTriggerLevel", OpenZWave::LogLevel_Error);
	Options::Get()->AddOptionInt("PollInterval", 500 );
	Options::Get()->AddOptionBool("IntervalBetweenPolls", TRUE);
	Options::Get()->AddOptionBool("ValidateValueChanges", TRUE);

	Options::Get()->Lock();

    Manager::Create();

    Manager::Get()->AddDriver("/dev/ttyACM0");

    return 0;
}
