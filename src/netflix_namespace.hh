/**
 * This file contains various constants and convenience functions related
 * to the Netflix challenge and the format of our data. All of these have
 * been gathered into a "netflix" namespace that should be used by other
 * files.
 *
 * Note that we're using data files where entries are in (user, movie)
 * order, **not** (movie, user) order.
 *
 */

#ifndef NETFLIX_NAMESPACE_HH
#define NETFLIX_NAMESPACE_HH

#include <vector>
#include <string>

using namespace std;

namespace netflix
{
    /* Constants */

    // Minimum and maximum possible ratings in this dataset.
    const int MIN_RATING = 1;
    const int MAX_RATING = 5;

    // Represents an entry with no rating (or rather, an unknown rating).
    const int NO_RATING = 0;

    // The total number of users and movies in the dataset.
    const int NUM_USERS = 458293;
    const int NUM_MOVIES = 17770;

    // The mean rating in the training set.
    const float MEAN_RATING_TRAINING_SET = 3.60951619727;
    
    // Name of the file containing all of the data.
    const string ALL_DATA_FN = "../data/um/all.dta";

    // Name of the file containing corresponding set indexes for "all.dta".
    const string ALL_IDX_FN = "../data/um/all.idx";

    // Name of the file containing "qual" set data only.
    const string QUAL_DATA_FN = "../data/um/qual.dta";

    // Name of the file containing the "N" matrix.
    const string N_FN = "../data/N.dta";
    
    // These indices represent the different kinds of data in all.dta.
    const int BASE_SET = 1;
    const int VALID_SET = 2;
    const int HIDDEN_SET = 3;
    const int PROBE_SET = 4;
    const int QUAL_SET = 5;
    
    // The delimiter used in our data files (e.g. in the data file containing
    // N).
    const string NETFLIX_FILES_DELIMITER = " ";
    
    
    /* Convenience functions */
    void splitIntoInts(const string &str, const string &delimiter,
                       vector<int>& output);
}


#endif // NETFLIX_NAMESPACE_HH