#include "../interface/Common.h"
#include "string"

int main( int argc, char **argv ) {
	std::string name = "shell";
	std::string report = "";
	bool is_name=false;
	for (int iA=1; iA!=argc; ++iA) {
		std::string this_arg = argv[iA];
		if (this_arg=="-n") {
			is_name = true;
			continue;
		}
		if (is_name) {
			name = this_arg;
			is_name = false;
			continue;
		}
		if (iA>1)
			report += " ";
		report += this_arg;
	}

	PError(name.c_str(),report.c_str());
}
