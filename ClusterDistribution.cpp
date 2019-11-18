#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <numeric>
#include <algorithm>
#include <iostream>
#include <random>
#include <fstream>
#include <string>
#include <fstream>
#include <iomanip> 
#include <chrono>
#include <thread>
#include <future>

const double deltaTMax = 4;
const unsigned int maxClusters = 22;
unsigned int nThreads = 2;
const double arrayInit = -1;

using Array4D = std::vector<std::vector<std::vector<std::vector<double>>>>;

std::vector<bool> threadActive(nThreads,false);

//File Processing
std::vector<std::string> split(const std::string& s, char delimiter)
{
	//dumb brute-force string splitter based on a delimiter
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (getline(tokenStream, token, delimiter))
	{
		if( token.length() > 0) // empty rows not allowed
		{
			tokens.push_back(token);
		}
	}
	return tokens;
}

class ClusterSolver
{
	std::vector<double> data;
	int N;
	Array4D cache;
	
	inline double recursiveDive(int m, int k, int n, int ell)
	{
		
		
		if (m < 0 )
		{
			return 0; //1
		}
		
		if (N - n == k)
		{
			if ((m == 1 && ell < n) || (m == 0 && ell == n))
			{
				 return 1; //3
			}
	        else
	        {
				return 0; //4
			}
		}
			
		if (cache[m][k][n][ell] !=arrayInit)
		{
			return cache[m][k][n][ell]; //2
		}
		
		
		double& value = cache[m][k][n][ell] = 0; 
		double prob = (float)k/(N - n);	
		if ( (k > 0))
		{
			if (ell == n)
			{
				value+=recursiveDive( m, k-1, n+1,n+1) * prob; //6
			}
			else
			{
				value += recursiveDive( m, k-1, n+1,ell) * prob; //7
			}	
		}
		
		if (data[ell] + deltaTMax < data[n])
		{
			value += recursiveDive(m-1, k, n+1,n) * (1.0  - prob); //8;
		}
		else
		{	
	            value += recursiveDive(m, k, n+1,n) * (1.0 -  prob); //9
		}
	    return value; //10
	}
	
	public: 
	
	ClusterSolver(std::vector<double> input) : data(input)
	{
		N = data.size();
		int truncMax = std::min(N+1,5);
		
		cache.resize(truncMax+1);
		for (int m = 0; m <= truncMax; ++m)
		{
			cache[m].resize(N + 1);
			for (int k = 0; k <= N; ++k)
			{	
				cache[m][k].resize(N + 1,std::vector<double>(N+1,arrayInit));
				//~ for (int n = 0; n <=N; ++n)
				//~ {	
					//~ cache[m][k][n].resize(N+1,arrayInit);
				//~ }
			}
		}
	}
	
	
	double P(int k)
	{
		//std::cout << "P has been called for k = " << k << std::endl;
		double sum = 0;
		for (int m = 0; m < 6; ++m)
		{
			sum += recursiveDive(m,k,0,0);
		}
		//std::cout << "P has finished" <<std::endl;
		return sum;
	}
};

void processLine(int lineID, std::string input, std::string * saver)
{
	const int dataOffset = 2;
	std::vector<std::string> line = split(input, ',');
	int N = line.size() -dataOffset;
	std::vector<double> data(N,0);
	
	for (int j = dataOffset; j < N+2; ++j)
	{
		double entry = stod(line[j]);
		data[j-dataOffset] = entry;
	}

	std::string id = std::to_string(lineID);
	while (id.size() < 8)
	{
		id = "0" + id;
	}
	saver->append(id);
	
	if (N <= 6)
	{
		for (int i = 0; i < N;++i)
		{
			saver->append(", 1.0");
		}
	}
	else
	{
		for (int i = 0; i < 6;++i)
		{
			saver->append(", 1.0");
		}
		
		ClusterSolver solver(data);
		for (int k = 6; k<=N;++k)
		{
			double pSum = solver.P(N - k);
			saver->append(", " + std::to_string(pSum));
		}
	}	
	saver->append("\n");
	
}

std::string convertTime(double secs)
{
	std::ostringstream time;
	int hours = floor(secs/3600.0);
	int mins = floor((secs - 3600.0*hours)/60.0);
	int cumulative = hours*3600 + mins*60;
	double roundingAccuracy = 4;
	secs = round((secs - cumulative)*(pow(10,roundingAccuracy)))/pow(10,roundingAccuracy);
	if (hours > 0)
	{
		time << hours << " hour";
		if (hours > 1)
		{
			time  << "s";
		}
		time << " ";
	}
	if (mins > 0)
	{
		time << mins << " minute";
		if (mins > 1)
		{
			time << "s";
		}
		time << " ";
	}
	if ((int)secs > 0)
	{
		time << (int)secs << " second";
		if ( (int)secs > 1)
		{
			time << "s";
		}
	}
	else
	{
		time << round(secs*100)/100 << " seconds ";
	}
	return time.str();
}

void printTimeSince(std::chrono::time_point<std::chrono::high_resolution_clock> start,int len,int total)
{
	auto finish = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = finish - start;
	double elapsedSecs = elapsed.count();
	
	double perc = round( 1000 * (float)len/total)/10;
	std::cout << "\033[A\33[2K\r[";
	std::cout << std::setw(1);
	for (int j = 0 ; j < floor(perc/5); ++j)
	{
		std::cout << "|";
	}
	for (int j = floor(perc/5); j < 20; ++j)
	{
		std::cout << " ";
	}
	std::cout << "] ";
	std::cout << "Elapsed: " << convertTime(elapsedSecs) << ", remaining: ";
	double secs = elapsedSecs*((float)total/len - 1);
	
	
	std::cout << convertTime(secs) << std::endl;
}

void blockProcess(std::vector<std::string> blocks,std::string * saveBlock, int nInBlock, int offset, int threadID)
{

	for (int i = 0; i < nInBlock; ++i)
	{
		processLine(offset + i,blocks[i], saveBlock);
	}
	
	threadActive[threadID] = false;
}
int totalLinesInFile = 0;

std::string getReadFile(int argc, char** argv)
{
	std::string fileName = "gaia_t_maps_1024.csv";
	if (argc > 1)
	{
		fileName = argv[1];
		std::cout << "Using provided " << fileName << " as processing target" <<std::endl;
	}
	
	std::ifstream testFile(fileName);
	if (!testFile.is_open())
	{
		std::cout << "\n\nERROR: Could not find datafile. Something terrible has occured.\n\n" << std::endl;
		exit(-1);
	}
	std::string rawLine;	
	while (getline(testFile,rawLine))
	{
		++totalLinesInFile;
	}
	testFile.close();
	std::cout << "A file with " << totalLinesInFile << " lines has been detected. Beginning analysis.\n\n" <<std::endl;
	return fileName;	
}

int main(int argc, char** argv)
{
	auto start = std::chrono::high_resolution_clock::now();


	// extract filename from input args
	std::string fileName = getReadFile(argc, argv);
	
	
	std::ifstream dataFile(fileName);

	std::string saveFileName = "ClusteringDistribution.dat";
	if (argc >2)
	{
		saveFileName = argv[2];
	}
	

	
	std::ofstream saveFile;
	saveFile.open(saveFileName);
	

	if (argc > 3)
	{
		nThreads = std::stoi(argv[3]);
	}
	
	std::vector<std::thread> persei(nThreads);
	int currentThread = 0;
	std::string rawLine;
	int blockSize = 3000;
	int currentOffset = 0;
	bool noThreadAssigned = false;
	std::vector<std::string> block(blockSize,"");
	std::vector<std::string> saveBlocks(nThreads,"");
	int blockID = 0;
	int count = 0;
	while (getline(dataFile,rawLine) )
	{
		while (noThreadAssigned==true)
		{
			for (int j = 0; j < nThreads; ++j)
			{

				if (threadActive[j]==false)
				{
					if (persei[j].joinable())
					{
						persei[j].join();
						saveFile << saveBlocks[j];
						saveBlocks[j] = "";
					}
					currentThread = j;
					currentOffset = count;
					j = nThreads+10;
					noThreadAssigned = false;
				}
			}
			
		}
	
		block[blockID] = rawLine;
		++blockID;
		
		//if block is full, get ready to launch!
		if (blockID == blockSize)
		{
			threadActive[currentThread] = true;
			persei[currentThread] = std::thread(blockProcess, block,&saveBlocks[currentThread],blockID,currentOffset,currentThread);
			blockID = 0;
			noThreadAssigned = true;
			//std::cout << "Thread launched at line " << count  << "/" << totalLines << std::endl;
		}
		++count;
		if (count % (totalLinesInFile / 200) == 0)
		{
			printTimeSince(start,count,totalLinesInFile);
		}
	}
	
	//add the remainder in
	if (blockID > 0)
	{
		persei[currentThread]= std::thread(blockProcess,block,&saveBlocks[currentThread],blockID,currentOffset,currentThread);
	}
	
	
	for (int j = 0; j < nThreads; ++j)
	{
		if (persei[j].joinable())
		{
			persei[j].join();
			saveFile << saveBlocks[j];
		}
	}
	
	
	saveFile.close();
	
	std::string sortcommand = "sort " + saveFileName + "  --field-separator=',' >> sorted_" + saveFileName; 
	system(sortcommand.c_str());
	
	printTimeSince(start,count,totalLinesInFile);
	return 0;	
}
