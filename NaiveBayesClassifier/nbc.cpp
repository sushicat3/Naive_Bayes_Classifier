#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>

using namespace std;

string usName = "US";
string rusName = "Russian";
string othName = "Other";

// set of predictors
char LETTERS [] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 
			    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};

// subset of predictors for each object
int predictors [26];

// arrays holding likelihoods* of each predictor for each class: US, RUSSIA, OTHER.
// *actually holds the number of times a predictor was observed in the city data for each class.
// *likelihood = llh_class[i] / 100
int us [26];
int russia [26];
int other [26];

void initLlh(int llh[]) {
	for (int i = 0; i < 26; ++i)
		llh[i] = 0;
}

void initAll() {
	initLlh(us);
	initLlh(russia);
	initLlh(other);
}

//-------------------------------------------------------------
//  PREDICTOR FUNCTIONS
//-------------------------------------------------------------
// - records the predictors "observed" for each city name
//	 in the global array predictors[]
//
//		X = set of predictors
// 		X = {Xa, Xb, ..., Xz}

void clearPredictors() {
	for (int i = 0; i < 26; ++i)
		predictors[i] = 0;
}

void observe( string city ) {
	clearPredictors();
	for (string::iterator it = city.begin(); it!=city.end(); ++it) {
		for (int i = 0; i < 26; ++i) {
			if ( (char) *it == LETTERS[i] ) {
				predictors[i] = 1;
			}
		}
	}
}

//-------------------------------------------------------------
//  LIKELIHOOD FUNCTIONS 
//-------------------------------------------------------------
// - calculates likelihoods for each predictor for the class of this file. 
//		
//		|C| = number of objects with classification C 
//		|C_Xi| = number of C objects with predictor Xi
//
//			P(Xi|C) = |C_Xi| / |C| = likelihood C object has predictor Xi
//

void updateLlh(int llh[]) {
	for (int i = 0; i < 26; ++i) {
		llh[i] += predictors[i];
	}
}

void likelihood( int llh[], string filename ) {
	
	ifstream infile;

    infile.open (filename.c_str(), ifstream::in);

    string city;
    	while (infile.good()) {
    		getline(infile, city);
   	    	observe(city);
			updateLlh(llh);
		}

    infile.close();
}

//-------------------------------------------------------------
//  POSTERIORI FUNCTION
//-------------------------------------------------------------
// - calculates posteriori (numerator only)
//
//		ex. P(C|Xi and not Xj) = [P(Xi|C) * P(not Xj|C)
//

double posteriori( string city, int arr[] ) {

	// prior is 0.5 because there are 100 cities of each type.
	double prob = 0.5;

	observe(city);
	for (int i = 0; i < 26; ++i) {
		if (predictors[i] == 1) {
			prob *= ( (double) arr[i]/100.0 );
		}
		else {
			prob *= (1 - ( (double) arr[i]/100.0 ) );
		}
	}

	return prob;
}

//-------------------------------------------------------------
//  MAP HYPOTHETHIS and CLASSIFICATION
//-------------------------------------------------------------
// - this function runs the classification step of NBC
//

void classify( string ifn, string ofn, int arrTru[], int arrFal[], string nameTru, string nameFal ) {

	ifstream infile;
	ofstream outfile;

	double pstPROB_TRU, pstPROB_FAL;
	bool acc = false;
	string nameMAP = nameTru;
	int acc_count = 0;
	int maxlength = 0;

    infile.open (ifn.c_str(), ifstream::in);
    outfile.open(ofn.c_str());

    outfile << setw(25) << "City"
    		<< setw(10) << "MAP"
    		<< setw(20) << nameTru + " posteriori"
    		<< setw(20) <<  nameFal + " posteriori" << endl;
    outfile << setfill('-') << setw(75) << "-" << endl;
    outfile << setfill(' ');

    string city;
    	while (infile.good()) {
    		getline(infile, city);

    		if (city.length() > maxlength) {
    			maxlength = city.length();
    		}

			pstPROB_TRU = posteriori(city, arrTru);
			pstPROB_FAL = posteriori(city, arrFal);

			if (pstPROB_TRU > pstPROB_FAL) acc = true;
			else acc = false;

			if (acc == true) {
				acc_count++;
				nameMAP = nameTru;
			}
			else nameMAP = nameFal;

			outfile << setw(25) << city 
					<< setw(10) << nameMAP 
					<< setw(20) << pstPROB_TRU
					<< setw(20) << pstPROB_FAL
					<< endl;
		}

		outfile << setfill('-') << setw(75) << "-" << endl;
		outfile << "Accuracy of this test: " << (acc_count/50.0)*100 << '%' << "\n";

    infile.close();
    outfile.close();
}

int main() {

	initAll();

	// step 1:
	cout << "Training..." << endl;
	likelihood(russia, "training/russiaCities100.txt");
	likelihood(us, "training/usCities100.txt");
	likelihood(other, "training/otherCities100.txt");

	// classify( string ifn, string ofn, int arrTru[], int arrFal[], string nameTru, string nameFal )
	//
	// - input file (the TRUE class)
	// - output file
	// - liklihood array of TRUE class
	// - likelihood array of FALSE class
	// - name of the TRUE class
	// - name of the FALSE class

	// step 2:	
	cout << "Experiment 1..." << endl;
	classify( "unknown/usCitiesNext50.txt", "experiments/exp1_USCities.txt", us, russia, usName, rusName);
	classify( "unknown/russiaCitiesNext50.txt", "experiments/exp1_RussiaCities.txt", russia, us, rusName, usName);

	cout << "Experiment 2..." << endl;
	classify( "unknown/russiaCitiesNext50.txt", "experiments/exp2_RussiaCities.txt", russia, other, rusName, othName);
	classify( "unknown/otherCitiesNext50.txt", "experiments/exp2_OtherCities.txt", other, russia, othName, rusName);

	cout << "Experiment 3..." << endl;
	classify( "unknown/usCitiesNext50.txt", "experiments/exp3_USCities.txt", us, other, usName, othName);
	classify( "unknown/otherCitiesNext50.txt", "experiments/exp3_OtherCities.txt", other, us, othName, usName);

    return 0;
}