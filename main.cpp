#include <algorithm>
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

constexpr int top = 5;
constexpr double PI = 3.14159265358979323846;

inline double mean( const vector<double>& v ) {
	double sum = 0;
	for( auto& x : v ) {
		sum += x;
	}
	return sum / v.size();
}

struct cords_pair {
	pair<double, double> base;
	pair<double, double> target;
};

class Country {
	string country_code;
	string country_name;
	string wb_income;
	string wb_region;
	vector<double> net_per_10K; // from 2015 to 2019, 5 values
public:

	Country( const string& code, const string& name, const string& income, const string& region, vector<double>&& net ) {
		country_code = code;
		country_name = name;
		wb_income = income;
		wb_region = region;
		net_per_10K = net;
	}

	//methods returning private variables
	string return_country_code() { return country_code; }
	string return_country_name() { return country_name; }
	string return_wb_income() { return wb_income; }
	string return_wb_region() { return wb_region; }
	vector<double> return_net_per_10K() { return net_per_10K; }
	double return_net_per_10K( const int i ) const { return net_per_10K[i - 2015]; }
};

class migration_Country : public Country {
	cords_pair cords;
	string target_country_code;
	string target_country_name;
	string target_wb_income;
	string target_wb_region;

public:
	migration_Country( Country&& country, const double& base_lat,
		const double& base_long, const double& target_lat, const double& target_long, const string& target_country_code, const string& target_country_name,
		const string& target_wb_income, const string& target_wb_region ) : Country( country ) {
		cords.base = make_pair( base_lat, base_long );
		cords.target = make_pair( target_lat, target_long );
		this->target_country_code = target_country_code;
		this->target_country_name = target_country_name;
		this->target_wb_income = target_wb_income;
		this->target_wb_region = target_wb_region;
	}

	cords_pair return_cords() { return cords; }
	string return_target_country_code() { return target_country_code; }
	string return_target_country_name() { return target_country_name; }
	string return_target_wb_income() { return target_wb_income; }
	string return_target_wb_region() { return target_wb_region; }
};

class Industry : public Country {
	string industry_name;
	string isic_section_name;
	int industry_id;
	char isic_section_index;

public:
	Industry( Country&& country, const string& industry_name,
		const string& isic_section_name, const int& industry_id, const char& isic_section_index ) : Country( country ) {
		this->industry_name = industry_name;
		this->isic_section_name = isic_section_name;
		this->industry_id = industry_id;
		this->isic_section_index = isic_section_index;
	}

	string return_industry_name() { return industry_name; }
	string return_isic_section_name() { return isic_section_name; }
	int return_industry_id() const { return industry_id; }
	char return_isic_section_index() const { return isic_section_index; }
};

class Skill : public Country {
	string skill_group_name;
	string skill_group_category;
	int skill_group_id;

public:

	Skill( Country&& country, const string& skill_group_name,
		const string& skill_group_category, const int& skill_group_id ) : Country( country ) {
		this->skill_group_name = skill_group_name;
		this->skill_group_category = skill_group_category;
		this->skill_group_id = skill_group_id;
	}

	string return_skill_group_name() { return skill_group_name; }
	string return_skill_group_category() { return skill_group_category; }
	int return_skill_group_id() const { return skill_group_id; }
};

// This function reads a CSV file and returns a vector of vectors of strings
vector<vector<string>> readCSV( const string& filename ) {
	// Open the file with a string as a parameter.
	ifstream file( filename );
	// Create an empty vector of vectors of strings.
	vector<vector<string>> data;
	// Create an empty string.
	string line;
	// While there is a line in the file.
	while( getline( file, line ) ) {
		// Create a string stream.
		stringstream lineStream( line );
		// Create an empty string.
		string cell;
		// Create an empty vector of strings.
		vector<string> row;
		// While there is a cell in the line.
		while( getline( lineStream, cell, ',' ) ) {
			// Add the cell to the row.
			row.push_back( cell );
		}
		// Add the row to the vector of rows.
		data.push_back( row );
	}
	// Return the vector of rows.
	return data;
}

//This function loads migration data from a 2D vector of strings into a vector of migration_Country objects.
//It first creates a map that stores the number of occurrences of each country code.
//Then, it iterates through the map and creates a Country object for each entry in the map.
//For each Country object, it stores the relevant data from the 2D vector of strings in the object.
//Finally, it creates a migration_Country object by moving the Country object into it and adding the remaining data from the 2D vector of strings.
//The migration_Country object is then added to the migration_countries vector.
void load_migration_data( vector<migration_Country>& migration_countries, const vector<vector<string>>& migration_data ) {
	map<string, int> codes;
	for( int i = 1; i < migration_data.size(); i++ ) {
		codes[migration_data[i][0]]++;
	}

	int line = 1;
	for( auto& c : codes ) {
		for( int i = 0; i < c.second; i++ ) {
			vector<double>temp_data( 5 );
			for( int k = 0; k < 5; k++ ) {
				if( migration_data[line][k + 12].empty() ) break;
				temp_data[k] = stod( migration_data[line][12 + k] );
			}

			Country temp_country( c.first, migration_data[line][1], migration_data[line][4], migration_data[line][5], std::move( temp_data ) );

			migration_countries.emplace_back( std::move( temp_country ), stoi( migration_data[line][2] ), stoi( migration_data[line][3] ),
				stoi( migration_data[line][8] ), stoi( migration_data[line][9] ), migration_data[line][6],
				migration_data[line][7], migration_data[line][10], migration_data[line][11] );

			line++;
		}
	}
}

//same as load_migration_data, but for industry data
void load_industries_data( vector<Industry>& industries, const vector<vector<string>>& industries_data ) {
	map<string, int> codes;
	for( int i = 1; i < industries_data.size(); i++ ) {
		codes[industries_data[i][0]]++;
	}

	int line = 1;
	for( auto& c : codes ) {
		for( int i = 0; i < c.second; i++ ) {
			vector<double>temp_data( 5 );
			for( int k = 0; k < 5; k++ ) {
				if( industries_data[line][8 + k].empty() ) break;
				temp_data[k] = stod( industries_data[line][8 + k] );
			}
			Country temp_country( c.first, industries_data[line][1], industries_data[line][2], industries_data[line][3], std::move( temp_data ) );

			industries.emplace_back( std::move( temp_country ), industries_data[line][7], industries_data[line][5],
				stoi( industries_data[line][6] ), industries_data[line][4][0] );

			line++;
		}
	}
}


//same as load_migration_data, but for skill data
void load_skills_data( vector<Skill>& skills, const vector<vector<string>>& skills_data ) {
	map<string, int> codes;
	for( int i = 1; i < skills_data.size(); i++ ) {
		codes[skills_data[i][0]]++;
	}

	int line = 1;
	for( auto& c : codes ) {
		for( int i = 0; i < c.second; i++ ) {
			vector<double>temp_data( 5 );
			for( int k = 0; k < 5; k++ ) {
				if( skills_data[line][7].empty() ) break;
				temp_data[k] = stod( skills_data[line][7 + k] );
			}

			Country temp_country( c.first, skills_data[line][1], skills_data[line][2], skills_data[line][3], std::move( temp_data ) );

			skills.emplace_back( std::move( temp_country ), skills_data[line][6], skills_data[line][5],
				stoi( skills_data[line][4] ) );

			line++;
		}
	}
}

//this function prints the data for a given country, using the country code as a parameter
//it creates a temp vector of pairs, where the former element is name and the latter mean of the net data
//it then sorts the vector by mean, using a lambda function to give the sort function a comparison function
//it then prints the data using global variable as the number of data to print
void write_top_industries( vector<Industry>& industries, const string& country ) {
	vector<pair<string, double>> top_industries;

	for( auto& i : industries ) {
		if( i.return_country_name() == country ) {
			top_industries.emplace_back( i.return_industry_name(), mean( i.return_net_per_10K() ) );
		}
	}

	if( top_industries.empty() )
	{
		cout << "No data" << endl;
		return;
	}

	sort( top_industries.begin(), top_industries.end(), []( const pair<string, double>& a, const pair<string, double>& b ) {
		return a.second > b.second;
		} );

	for( int i = 0; i < top && i < top_industries.size(); i++ ) {
		cout << top_industries[i].first << " " << top_industries[i].second << endl;
	}
}


//same as write_top_industries, but for skills
void write_top_skills( vector<Skill>& skills, const string& country )
{
	vector<pair<string, double>> top_skills;

	for( auto& s : skills ) {
		if( s.return_country_name() == country ) {
			top_skills.emplace_back( s.return_skill_group_name(), mean( s.return_net_per_10K() ) );
		}
	}

	if( top_skills.empty() ) {
		cout << "No data" << endl;
		return;
	}

	sort( top_skills.begin(), top_skills.end(), []( const pair<string, double>& a, const pair<string, double>& b ) {
		return a.second > b.second;
		} );

	for( int i = 0; i < top && i < top_skills.size(); i++ ) {
		cout << top_skills[i].first << " " << top_skills[i].second << endl;
	}

}

//this function calculates the distance between two countries, using their coordinates as parameters
double distance_in_km( const pair<double, double> coord1, const pair <double, double> coord2 ) {
	constexpr double EARTH_RADIUS_KM = 6371.0;
	double lat1 = coord1.first;
	double lon1 = coord1.second;
	double lat2 = coord2.first;
	double lon2 = coord2.second;

	double lat_diff = (lat2 - lat1) * PI / 180.0;
	double lon_diff = (lon2 - lon1) * PI / 180.0;
	lat1 = lat1 * PI / 180.0;
	lat2 = lat2 * PI / 180.0;
	lon1 = lon1 * PI / 180.0;
	lon2 = lon2 * PI / 180.0;

	double a = sin( lat_diff / 2 ) * sin( lat_diff / 2 ) +
		cos( lat1 ) * cos( lat2 ) * sin( lon_diff / 2 ) * sin( lon_diff / 2 );
	double c = 2 * atan2( sqrt( a ), sqrt( 1 - a ) );

	return EARTH_RADIUS_KM * c;
}

//same as write_top_industries, but for direction
void write_top_directions( vector<migration_Country>& migration_countries, const string& country )
{
	struct temp_country {
		string name;
		double mean;
		cords_pair cords;
	};

	vector<temp_country> top_directions;

	for( auto& m : migration_countries ) {
		if( m.return_country_name() == country ) {
			top_directions.emplace_back( temp_country{ m.return_target_country_name(), mean( m.return_net_per_10K() ), m.return_cords() } );
		}
	}

	if( top_directions.empty() ) {
		cout << "No data" << endl;
		return;
	}


	//we sort by mean value
	sort( top_directions.begin(), top_directions.end(), []( const temp_country& a, const temp_country& b ) {
		return a.mean > b.mean;
		} );


	for( int i = 0; i < top && i < top_directions.size(); i++ ) {
		cout << fixed << setprecision( 2 );
		cout << left << setw( 16 ) << top_directions[i].name << ' ' << top_directions[i].mean << "  Distance: " << distance_in_km( top_directions[i].cords.base, top_directions[i].cords.target ) << "km" << endl;
	}

}


int main() {

	//we create vectors for each type of data
	vector<migration_Country> migration_countries;
	vector<Industry> industries;
	vector<Skill> skills;

	//we load the data from the files
	vector<vector<string>> migration_data = readCSV( "country_migration_public.csv" );
	vector<vector<string>> industries_data = readCSV( "industry_migration_public.csv" );
	vector<vector<string>> skills_data = readCSV( "skill_migration_public.csv" );

	//we process the data
	load_migration_data( migration_countries, migration_data );
	load_industries_data( industries, industries_data );
	load_skills_data( skills, skills_data );

	//clear the vectors, as we don't need them anymore
	migration_data.clear();
	industries_data.clear();
	skills_data.clear();

	//we create a map, so we can print names of countries
	map <string, int> countries;
	for( auto& c : migration_countries )
	{
		countries[c.return_country_name()]++;
	}

	//list all countries
	for( auto& c : countries ) {
		cout << c.first << endl;
	}
	cout << "Number of countries: " << countries.size() << endl << endl;

	while( 1 )
	{
		string country;
		cout << "\n\n" << "If you want to exit the program press 1 and then Enter" << endl;
		cout << "If you want to list all countries press 2 and then Enter" << endl;
		cout << "Enter country name: " << endl;
		getline( cin, country );
		if( country == "1" ) break;
		if( country == "2" )
		{
			for( auto& c : countries ) {
				cout << c.first << endl;
			}
			continue;
		}
		//find country in the vector
		auto it = find_if( migration_countries.begin(), migration_countries.end(), [&country]( migration_Country& c ) {
			return c.return_country_name() == country;
			} );

		//if the country was not found, write an error message
		if( it == migration_countries.end() ) {
			cout << "No data for this country" << endl;
			continue;
		}

		cout << "\nCountry region name: " << it->return_wb_region() << endl;
		cout << country << " is in the " << it->return_wb_income() << " income group" << endl;
		cout << "\n\nTop industries : " << endl;
		write_top_industries( industries, country );
		cout << "\n\nTop skills: " << endl;
		write_top_skills( skills, country );
		cout << "\n\nTop directions: " << endl;
		write_top_directions( migration_countries, country );
	}
}


