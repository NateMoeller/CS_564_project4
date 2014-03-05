#include "load.h"
#include <sqlite3.h>
#include <stdlib.h>

using namespace std;

sqlite3 *database; //database to create
sqlite3_stmt *statement; //current statement being executed

/* loadInfo
 *
 * This method is given a path to load the file, the name of the table to insert the data
 * and the number of lines to parse. It will go line by line and get each value to be inserted.
 * The method then loads the word into the vector called variables. After everything is parsed
 * the method will loop through the variables vector and insert them into the table.
 *
 * @param path where to load the file from
 * @param tableName where to insert the parsed data
 * @param numLines determines when we should stop parsing
 */

void loadInfo(char* path, string tableName, int numLines){
	vector<string> variables; //variables to parse
	string query = "INSERT INTO " + tableName + "("; //insert query
	string line; //line to parse
	int linenum = 0;
	int commaPos = 0;
	int varNum = 0;
	ifstream file(path);
	if(file.is_open()){
		//begin a transaction
		sqlite3_exec(database, "BEGIN", 0, 0, 0);
		while(file.good()){
			getline(file, line);
			if(linenum == 0){
				query += line + ") VALUES(";
			}
			else if(linenum < numLines){
				while(true){
					if(line.at(0) == '"'){
						//we want to parse the quoted string, not until comma
						line = line.substr(1, line.length());
						int quotePos = line.find('"');
						string word = line.substr(0, quotePos);
						variables.push_back(word);
						varNum++;
						line = line.substr(quotePos+2, line.length());
					}
					int commaPos = line.find(",");
					string word = line.substr(0, commaPos);
					variables.push_back(word);
					varNum++;
					line = line.substr(commaPos+1, line.length());
					if(commaPos == string::npos){
						break;
					}
				}
				//fix the query
				if(linenum == 1){
					for(int i = 0; i < varNum; i++){
						if(i == varNum-1){
							query += "?);";
						}
						else{
							query += "?, ";
						}
					}
				}

				int rc = sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0);
				if(rc == SQLITE_OK){
					// bind the values
					for(int i = 0; i < variables.size(); i++){
						if(variables.at(i) == "" || isalpha(variables.at(i).at(0))){
							//dont convert
							sqlite3_bind_text(statement, i+1, variables.at(i).c_str(), -1, 0);
						}
						else{
							double value = atof(variables.at(i).c_str());
							sqlite3_bind_double(statement, i+1, value);
						}
					}
					// commit
					sqlite3_step(statement);
					sqlite3_finalize(statement);
				}
			}
			//clear and update variables
			linenum++;
			commaPos = 0;
			varNum = 0;
			variables.clear();
		}
		sqlite3_exec(database, "COMMIT", 0, 0, 0);
		file.close();
	}
	else{
		cout << "File failed to open" << endl;
	}
}

int main() {

	//create a database
	sqlite3_open("census.db", &database);

	//check if division table exists, if it does drop it
	char* dropQuery = "DROP TABLE IF EXISTS DIVISION";
	if(sqlite3_prepare_v2(database, dropQuery, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
	//create division table
	char* query = "CREATE TABLE DIVISION (DIVISION_Cd NUMBER PRIMARY KEY, DIVISION_Name String);";
	if(sqlite3_prepare_v2(database, query, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
	//load division data
	string line;
	int lineNum = 0;
	ifstream textFile ("src/Division.txt");
	if(textFile.is_open()){
		while(textFile.good()){
			getline (textFile,line);
			int commaPos = line.find(",");
			string word = line.substr(0, commaPos);
			line = line.substr(commaPos+1, line.length()-3);
			if(lineNum > 0 && lineNum <=10){
				int value = atoi(word.c_str());
				char* insertQuery = "INSERT INTO DIVISION (DIVISION_Cd, DIVISION_Name) VALUES (?, ?);";
				int rc = sqlite3_prepare_v2(database, insertQuery, -1, &statement, 0);
				if(rc == SQLITE_OK){
					// bind the value
					sqlite3_bind_int(statement, 1, value);
					sqlite3_bind_text(statement, 2, line.c_str(), -1, 0);

					// commit
					sqlite3_step(statement);
					sqlite3_finalize(statement);
				}
			}
			lineNum++;
		}
		textFile.close();
	}

	//check if region table exists, if it does drop it
	dropQuery = "DROP TABLE IF EXISTS REGION";
	if(sqlite3_prepare_v2(database, dropQuery, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
	//create region table
	query = "CREATE TABLE REGION (REGION_Cd NUMBER PRIMARY KEY, REGION_Desc String);";
	if(sqlite3_prepare_v2(database, query, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}

	//load region data
	lineNum = 0;
	ifstream textFileRegion ("src/Region.txt");
	if(textFileRegion.is_open()){
		while(textFileRegion.good()){
			getline (textFileRegion,line);
			int commaPos = line.find(",");
			string word = line.substr(0, commaPos);
			line = line.substr(commaPos+1, line.length()-3);
			if(lineNum > 0 && lineNum < 6){
				int value = atoi(word.c_str());
				char* insertQuery = "INSERT INTO REGION (REGION_Cd, REGION_Desc) VALUES (?, ?);";
				int rc = sqlite3_prepare_v2(database, insertQuery, -1, &statement, 0);
				if(rc == SQLITE_OK){
					// bind the value
					sqlite3_bind_int(statement, 1, value);
					sqlite3_bind_text(statement, 2, line.c_str(), -1, 0);

					// commit
					sqlite3_step(statement);
					sqlite3_finalize(statement);
				}

			}
			lineNum++;
		}
		textFileRegion.close();
	}

	//check if ORIGIN table exists, if it does drop it
	dropQuery = "DROP TABLE IF EXISTS ORIGIN";
	if(sqlite3_prepare_v2(database, dropQuery, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
	//create origin table
	query = "CREATE TABLE ORIGIN (ORIGIN_Cd NUMBER PRIMARY KEY, ORIGIN_Desc String);";
	if(sqlite3_prepare_v2(database, query, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}

	//load origin data
	lineNum = 0;
	ifstream textFileOrigin ("src/Origin.txt");
	if(textFileOrigin.is_open()){
		while(textFileOrigin.good()){
			getline (textFileOrigin,line);
			int commaPos = line.find(",");
			string word = line.substr(0, commaPos);
			if(lineNum == 3){
				line = line.substr(commaPos+1, line.length()-2);
			}
			else{
				line = line.substr(commaPos+1, line.length()-3);
			}
			if(lineNum > 0 && lineNum < 4){
				int value = atoi(word.c_str());
				char* insertQuery = "INSERT INTO ORIGIN (ORIGIN_Cd, ORIGIN_Desc) VALUES (?, ?);";
				int rc = sqlite3_prepare_v2(database, insertQuery, -1, &statement, 0);
				if(rc == SQLITE_OK){
					// bind the value
					sqlite3_bind_int(statement, 1, value);
					sqlite3_bind_text(statement, 2, line.c_str(), -1, 0);

					// commit
					sqlite3_step(statement);
					sqlite3_finalize(statement);
				}

			}
			lineNum++;
		}
		textFileOrigin.close();
	}

	//check if RACE table exists, if it does drop it
	dropQuery = "DROP TABLE IF EXISTS RACE";
	if(sqlite3_prepare_v2(database, dropQuery, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
	//create race table
	query = "CREATE TABLE RACE (RACE_Cd NUMBER PRIMARY KEY, RACE_Desc String);";
	if(sqlite3_prepare_v2(database, query, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}

	//populate race table
	lineNum = 0;
	ifstream textFileRace ("src/Race.txt");
	if(textFileRace.is_open()){
		while(textFileRace.good()){
			getline (textFileRace,line);
			int commaPos = line.find(",");
			string word = line.substr(0, commaPos);
			if(lineNum == 6){
				line = line.substr(commaPos+1, line.length()-2);
			}
			else{
				line = line.substr(commaPos+1, line.length()-3);
			}
			if(lineNum > 0 && lineNum < 8){
				int value = atoi(word.c_str());
				char* insertQuery = "INSERT INTO RACE (RACE_Cd, RACE_Desc) VALUES (?, ?);";
				int rc = sqlite3_prepare_v2(database, insertQuery, -1, &statement, 0);
				if(rc == SQLITE_OK){
					// bind the value
					sqlite3_bind_int(statement, 1, value);
					sqlite3_bind_text(statement, 2, line.c_str(), -1, 0);

					// commit
					sqlite3_step(statement);
					sqlite3_finalize(statement);
				}

			}
			lineNum++;
		}
		textFileRace.close();
	}

	//check if SEX table exists, if it does drop it
	dropQuery = "DROP TABLE IF EXISTS SEX";
	if(sqlite3_prepare_v2(database, dropQuery, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
	//create sex table
	query = "CREATE TABLE SEX (SEX_Cd NUMBER PRIMARY KEY, SEX_Desc String);";
	if(sqlite3_prepare_v2(database, query, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
	//populate sex table
	lineNum = 0;
	ifstream textFileSex ("src/Sex.txt");
	if(textFileSex.is_open()){
		while(textFileSex.good()){
			getline (textFileSex,line);
			int commaPos = line.find(",");
			string word = line.substr(0, commaPos);
			if(lineNum == 3){
				line = line.substr(commaPos+1, line.length()-2);
			}
			else{
				line = line.substr(commaPos+1, line.length()-3);
			}
			if(lineNum > 0 && lineNum < 4){
				int value = atoi(word.c_str());
				char* insertQuery = "INSERT INTO SEX (SEX_Cd, SEX_Desc) VALUES (?, ?);";
				int rc = sqlite3_prepare_v2(database, insertQuery, -1, &statement, 0);
				if(rc == SQLITE_OK){
					// bind the value
					sqlite3_bind_int(statement, 1, value);
					sqlite3_bind_text(statement, 2, line.c_str(), -1, 0);

					// commit
					sqlite3_step(statement);
					sqlite3_finalize(statement);
				}

			}
			lineNum++;
		}
		textFileSex.close();
	}

	//check if SUMLEV table exists, if it does drop it
	dropQuery = "DROP TABLE IF EXISTS SUMLEV";
	if(sqlite3_prepare_v2(database, dropQuery, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
	//create SUMLEV table
	query = "CREATE TABLE SUMLEV (SUMLEV_Cd NUMBER PRIMARY KEY, SUMLEV_Desc String);";
	if(sqlite3_prepare_v2(database, query, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
	//populate SUMLEV table
	lineNum = 0;
	ifstream textFileSumlev ("src/Sumlev.txt");
	if(textFileSumlev.is_open()){
		while(textFileSumlev.good()){
			getline (textFileSumlev,line);
			int commaPos = line.find(",");
			string word = line.substr(0, commaPos);

			line = line.substr(commaPos+1, line.length()-5);

			if(lineNum > 0 && lineNum < 11){
				int value = atoi(word.c_str());
				char* insertQuery = "INSERT INTO SUMLEV (SUMLEV_Cd, SUMLEV_Desc) VALUES (?, ?);";
				int rc = sqlite3_prepare_v2(database, insertQuery, -1, &statement, 0);
				if(rc == SQLITE_OK){
					// bind the value
					sqlite3_bind_int(statement, 1, value);
					sqlite3_bind_text(statement, 2, line.c_str(), -1, 0);

					// commit
					sqlite3_step(statement);
					sqlite3_finalize(statement);
				}

			}
			lineNum++;
		}
		textFileSumlev.close();
	}

	//check if HU_UNIT_STATE_LEVEL table exists, if it does drop it
	dropQuery = "DROP TABLE IF EXISTS HU_UNIT_STATE_LEVEL";
	if(sqlite3_prepare_v2(database, dropQuery, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
	//create HU_UNIT_STATE_LEVEL table
	query = "CREATE TABLE HU_UNIT_STATE_LEVEL (SUMLEV NUMBER, STATE NUMBER PRIMARY KEY, REGION NUMBER, "
			"DIVISION NUMBER, STNAME String, HUCENSUS2010 NUMBER, HUESTBASE2010 NUMBER, HUEST_2010 NUMBER, "
			"HUEST_2011 NUMBER, FOREIGN KEY(SUMLEV) REFERENCES SUMLEV(SUMLEV_Cd), "
			"FOREIGN KEY(REGION) REFERENCES REGION(REGION_Cd), FOREIGN KEY(DIVISION) REFERENCES DIVISION(DIVISION_Cd));";
	if(sqlite3_prepare_v2(database, query, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
	loadInfo("src/Housing_Units_State_Level.txt", "HU_UNIT_STATE_LEVEL", 52);

	//check if POP_EST_METRO_MICRO table exists, if it does drop it
	dropQuery = "DROP TABLE IF EXISTS POP_EST_METRO_MICRO";
	if(sqlite3_prepare_v2(database, dropQuery, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
	//create POP_EST_METRO_MICRO table
	query = "CREATE TABLE POP_EST_METRO_MICRO (CBSA NUMBER, MDIV NUMBER, STCOU NUMBER, NAME String, "
			"LSAD String, CENSUS2010POP NUMBER, ESTIMATESBASE2010 NUMBER, POPESTIMATE2010 NUMBER, "
			"POPESTIMATE2011 NUMBER, NPOPCHG_2010 NUMBER, NPOPCHG_2011 NUMBER, NATURALINC2010 NUMBER, NATURALINC2011 NUMBER,"
			"BIRTHS2010 NUMBER, BIRTHS2011 NUMBER, DEATHS2010 NUMBER, DEATHS2011 NUMBER, NETMIG2010 NUMBER, NETMIG2011 NUMBER, "
			"INTERNATIONALMIG2010 NUMBER, INTERNATIONALMIG2011 NUMBER, DOMESTICMIG2010 NUMBER, DOMESTICMIG2011 NUMBER, RESIDUAL2010 NUMBER, "
			"RESIDUAL2011 NUMBER, PRIMARY KEY(NAME, LSAD));";
	if(sqlite3_prepare_v2(database, query, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
	loadInfo("src/Pop_Estimate_Metro_Micro.txt", "POP_EST_METRO_MICRO", 2763);


	//check if POP_EST_PR_MUN_SEX_AGE table exists, if it does drop it
	dropQuery = "DROP TABLE IF EXISTS POP_EST_PR_MUN_SEX_AGE";
	if(sqlite3_prepare_v2(database, dropQuery, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
	//create POP_EST_PR_MUN_SEX_AGE table
	query = "CREATE TABLE POP_EST_PR_MUN_SEX_AGE (SUMLEV NUMBER, MUNICIPIO NUMBER, NAME String, YEAR NUMBER, "
			"POPESTIMATE NUMBER, POPEST_MALE NUMBER, POPEST_FEM NUMBER, UNDER5_TOT NUMBER, UNDER5_MALE NUMBER, "
			"UNDER5_FEM NUMBER, AGE513_TOT NUMBER, AGE513_MALE NUMBER, AGE513_FEM NUMBER, AGE1417_TOT NUMBER, "
			"AGE1417_MALE NUMBER, AGE1417_FEM NUMBER, AGE1824_TOT NUMBER, AGE1824_MALE NUMBER, AGE1824_FEM NUMBER, "
			"AGE16PLUS_TOT NUMBER, AGE16PLUS_MALE NUMBER, AGE16PLUS_FEM NUMBER, AGE18PLUS_TOT NUMBER, AGE18PLUS_MALE NUMBER, "
			"AGE18PLUS_FEM NUMBER, AGE1544_TOT NUMBER, AGE1544_MALE NUMBER, AGE1544_FEM NUMBER, AGE2544_TOT NUMBER, "
			"AGE2544_MALE NUMBER, AGE2544_FEM NUMBER, AGE4564_TOT NUMBER, AGE4564_MALE NUMBER, AGE4564_FEM NUMBER, "
			"AGE65PLUS_TOT NUMBER, AGE65PLUS_MALE NUMBER, AGE65PLUS_FEM NUMBER, AGE85PLUS_TOT NUMBER, AGE85PLUS_MALE NUMBER, "
			"AGE85PLUS_FEM NUMBER, MEDIAN_AGE_TOT NUMBER, MEDIAN_AGE_MALE NUMBER, MEDIAN_AGE_FEM NUMBER, PRIMARY KEY(MUNICIPIO, YEAR), FOREIGN KEY(SUMLEV) REFERENCES SUMLEV(SUMLEV_Cd));";
	if(sqlite3_prepare_v2(database, query, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
	loadInfo("src/Pop_Estimate_PR_Mun_Sex_Age.txt", "POP_EST_PR_MUN_SEX_AGE", 313);

	//check if POP_EST_PR_MUNICIPIOS table exists, if it does drop it
	dropQuery = "DROP TABLE IF EXISTS POP_EST_PR_MUNICIPIOS";
	if(sqlite3_prepare_v2(database, dropQuery, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
	//create POP_EST_PR_MUNICIPIOS table
	query = "CREATE TABLE POP_EST_PR_MUNICIPIOS (SUMLEV NUMBER,MUNICIPIO NUMBER PRIMARY KEY,NAME String, "
			"ESTIMATESBASE2010 NUMBER,POPESTIMATE2010 NUMBER,POPESTIMATE2011 NUMBER,NPOPCHG2010 REAL, "
			"NPOPCHG2011 REAL,PPOPCHG2010 REAL,PPOPCHG2011 REAL,SRANK_ESTBASE2010 NUMBER, SRANK_POPEST2010 NUMBER,"
			"SRANK_POPEST2011 NUMBER, SRANK_NPCHG2010 NUMBER, SRANK_NPCHG2011 NUMBER, SRANK_PPCHG2010 NUMBER, SRANK_PPCHG2011 NUMBER);";
	if(sqlite3_prepare_v2(database, query, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
	loadInfo("src/Pop_Estimate_PR_Municipios.txt", "POP_EST_PR_MUNICIPIOS", 80);

	//check if POP_EST_PR_SEX_AGE table exists, if it does drop it
	dropQuery = "DROP TABLE IF EXISTS POP_EST_PR_SEX_AGE";
	if(sqlite3_prepare_v2(database, dropQuery, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
	//create POP_EST_PR_SEX_AGE table
	query = "CREATE TABLE POP_EST_PR_SEX_AGE (SUMLEV NUMBER, STATE NUMBER, NAME String, SEX NUMBER, AGE NUMBER, "
			"CENSUS2010POP NUMBER, ESTIMATESBASE2010 NUMBER, POPESTIMATE2010 NUMBER, POPESTIMATE2011 NUMBER, PRIMARY KEY(SEX, AGE), "
			"FOREIGN KEY(SUMLEV) REFERENCES SUMLEV(SUMLEV_Cd), FOREIGN KEY(SEX) REFERENCES SEX(SEX_Cd));";
	if(sqlite3_prepare_v2(database, query, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
	loadInfo("src/Pop_Estimate_PR_Sex_Age.txt", "POP_EST_PR_SEX_AGE", 262);


	//check if POP_EST_STATE_AGE_SEX_RACE_ORIGIN table exists, if it does drop it
	dropQuery = "DROP TABLE IF EXISTS POP_EST_STATE_AGE_RACE_ORIGIN";
	if(sqlite3_prepare_v2(database, dropQuery, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
	//create POP_EST_STATE_AGE_SEX_RACE_ORIGIN table
	query = "CREATE TABLE POP_EST_STATE_AGE_SEX_RACE_ORIGIN (SUMLEV NUMBER, REGION NUMBER, "
			"DIVISION NUMBER, STATE NUMBER, SEX NUMBER, ORIGIN NUMBER, RACE NUMBER, AGE NUMBER, CENSUS2010POP NUMBER, "
			"ESTIMATESBASE2010 NUMBER, POPESTIMATE2010 NUMBER, POPESTIMATE2011 NUMBER, PRIMARY KEY(STATE, SEX, ORIGIN, RACE, AGE), "
			"FOREIGN KEY(SUMLEV) REFERENCES SUMLEV(SUMLEV_Cd), FOREIGN KEY(REGION) REFERENCES REGION (REGION_Cd), "
			"FOREIGN KEY(DIVISION) REFERENCES DIVISION(DIVISION_Cd), FOREIGN KEY(SEX) REFERENCES SEX(SEX_Cd), "
			"FOREIGN KEY(ORIGIN) REFERENCES ORIGIN(ORIGIN_Cd), FOREIGN KEY(RACE) REFERENCES RACE (RACE_Cd));";
	if(sqlite3_prepare_v2(database, query, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
	loadInfo("src/Pop_Estimate_State_Age_Sex_Race_Origin.txt", "POP_EST_STATE_AGE_SEX_RACE_ORIGIN", 236845);


	//check if POP_EST_CITIES_TOWNS table exists, if it does drop it
	dropQuery = "DROP TABLE IF EXISTS POP_EST_CITIES_TOWNS";
	if(sqlite3_prepare_v2(database, dropQuery, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
	//create POP_EST_CITIES_TOWNS table
	query = "CREATE TABLE POP_EST_CITIES_TOWNS (SUMLEV NUMBER, STATE NUMBER, COUNTY NUMBER, PLACE NUMBER, COUSUB NUMBER,"
			"CONCIT NUMBER, NAME String, STNAME String, CENSUS2010POP NUMBER, ESTIMATESBASE2010 NUMBER, POPESTIMATE2010 NUMBER, "
			"POPESTIMATE2011 NUMBER, PRIMARY KEY(STATE, COUNTY, PLACE, COUSUB, CONCIT), FOREIGN KEY(SUMLEV) REFERENCES SUMLEV(SUMLEV_Cd));";
	if(sqlite3_prepare_v2(database, query, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
	loadInfo("src/Pop_Estimate_Cities_Towns.txt", "POP_EST_CITIES_TOWNS", 81747);


	//check if POP_EST_STATE_COUNTY table exists, if it does drop it
	dropQuery = "DROP TABLE IF EXISTS POP_EST_STATE_COUNTY";
	if(sqlite3_prepare_v2(database, dropQuery, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
	//create POP_EST_STATE_COUNTY table
	query = "CREATE TABLE POP_EST_STATE_COUNTY (SUMLEV NUMBER, REGION NUMBER, DIVISION NUMBER, STATE NUMBER, "
			"COUNTY NUMBER, STNAME String, CTYNAME String, CENSUS2010POP NUMBER, ESTIMATESBASE2010 NUMBER, "
			"POPESTIMATE2010 NUMBER, POPESTIMATE2011 NUMBER, POPESTIMATE2012 NUMBER, NPOPCHG_2010 NUMBER, NPOPCHG_2011 NUMBER, "
			"NPOPCHG_2012 NUMBER, BIRTHS2010 NUMBER, BIRTHS2011 NUMBER, BIRTHS2012 NUMBER, DEATHS2010 NUMBER, "
			"DEATHS2011 NUMBER, DEATHS2012 NUMBER, NATURALINC2010 NUMBER, NATURALINC2011 NUMBER, NATURALINC2012 NUMBER, "
			"INTERNATIONALMIG2010 NUMBER, INTERNATIONALMIG2011 NUMBER, INTERNATIONALMIG2012 NUMBER, DOMESTICMIG2010 NUMBER, "
			"DOMESTICMIG2011 NUMBER, DOMESTICMIG2012 NUMBER, NETMIG2010 NUMBER, NETMIG2011 NUMBER, NETMIG2012 NUMBER, "
			"RESIDUAL2010 NUMBER, RESIDUAL2011 NUMBER, RESIDUAL2012 NUMBER, GQESTIMATESBASE2010 NUMBER, GQESTIMATES2010 NUMBER, "
			"GQESTIMATES2011 NUMBER, GQESTIMATES2012 NUMBER, RBIRTH2011 REAL, RBIRTH2012 REAL, RDEATH2011 REAL, RDEATH2012 REAL, "
			"RNATURALINC2011 REAL, RNATURALINC2012 REAL, RINTERNATIONALMIG2011 REAL, RINTERNATIONALMIG2012 REAL, RDOMESTICMIG2011 REAL, "
			"RDOMESTICMIG2012 REAL, RNETMIG2011 REAL, RNETMIG2012 REAL, PRIMARY KEY(STATE, COUNTY), "
			"FOREIGN KEY(SUMLEV) REFERENCES SUMLEV(SUMLEV_Cd), FOREIGN KEY(REGION) REFERENCES REGION (REGION_Cd), "
			"FOREIGN KEY(DIVISION) REFERENCES DIVISION(DIVISION_Cd));";
	if(sqlite3_prepare_v2(database, query, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
	loadInfo("src/Pop_Estimate_State_County.txt", "POP_EST_STATE_COUNTY", 3195);

	//check if POP_EST_NATION_STATE_PR table exists, if it does drop it
	dropQuery = "DROP TABLE IF EXISTS POP_EST_NATION_STATE_PR";
	if(sqlite3_prepare_v2(database, dropQuery, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
	//create POP_EST_NATION_STATE_PR table
	query = "CREATE TABLE POP_EST_NATION_STATE_PR (SUMLEV NUMBER, REGION NUMBER, DIVISION NUMBER, "
			"STATE NUMBER, NAME String PRIMARY KEY, CENSUS2010POP NUMBER, ESTIMATESBASE2010 NUMBER, "
			"POPESTIMATE2010 NUMBER, POPESTIMATE2011 NUMBER, POPESTIMATE2012 NUMBER, NPOPCHG_2010 NUMBER, "
			"NPOPCHG_2011 NUMBER, NPOPCHG_2012 NUMBER, BIRTHS2010 NUMBER, BIRTHS2011 NUMBER, BIRTHS2012 NUMBER, "
			"DEATHS2010 NUMBER, DEATHS2011 NUMBER, DEATHS2012 NUMBER, NATURALINC2010 NUMBER, NATURALINC2011 NUMBER, "
			"NATURALINC2012 NUMBER, INTERNATIONALMIG2010 NUMBER, INTERNATIONALMIG2011 NUMBER, INTERNATIONALMIG2012 NUMBER, "
			"DOMESTICMIG2010 NUMBER, DOMESTICMIG2011 NUMBER, DOMESTICMIG2012 NUMBER, NETMIG2010 NUMBER, NETMIG2011 NUMBER, "
			"NETMIG2012 NUMBER, RESIDUAL2010 NUMBER, RESIDUAL2011 NUMBER, RESIDUAL2012 NUMBER, RBIRTH2011 REAL, RBIRTH2012 REAL, "
			"RDEATH2011 REAL, RDEATH2012 REAL, RNATURALINC2011 REAL, RNATURALINC2012 REAL, RINTERNATIONALMIG2011 REAL, RINTERNATIONALMIG2012 REAL, "
			"RDOMESTICMIG2011 REAL, RDOMESTICMIG2012 REAL, RNETMIG2011 REAL, RNETMIG2012 REAL, "
			"FOREIGN KEY(SUMLEV) REFERENCES SUMLEV(SUMLEV_Cd), FOREIGN KEY(REGION) REFERENCES REGION(REGION_Cd), "
			"FOREIGN KEY(DIVISION) REFERENCES DIVISION (DIVISION_Cd)) ;";
	if(sqlite3_prepare_v2(database, query, -1, &statement, 0) == SQLITE_OK){
		int cols = sqlite3_column_count(statement);
		int result = 0;
		result = sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
	loadInfo("src/Pop_Estimate_Nation_State_PR.txt", "POP_EST_NATION_STATE_PR", 58);


	//close database file
	sqlite3_close(database);
	return 0;
}
