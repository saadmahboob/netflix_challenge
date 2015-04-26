#ifndef NDEBUG
#include <chrono>
#include <iostream>

using namespace std::chrono;
#endif

#include <timesvdpp.hh>


/** 
 * A constructor for this run of Time-SVD++. Note that this constructor
 * does not make use of previously cached data, and will need to be
 * trained!
 *
 * @param numUsers:             Number of users in the entire data set (not
 *                              just training set).
 * @param numItems:             Number of items in the entire data set (not
 *                              just training set).
 * @param numTimes:             Number of times (i.e. days) in the entire
 *                              data set (not just the training set).
 * @param meanRating:           The mean rating of items in the training
 *                              set.
 * @param numFactors:           The number of factors to use for the SVD.
 * @param numIterations:        The number of iterations to use for Time-
 *                              SVD++.
 * @param numTimeBins:          The number of time bins to use for the
 *                              time-dependent item bias.
 * @param fileNameN:            Name of the file that contains the
 *                              information needed to populate the N
 *                              mapping. This should be a plain text .dta
 *                              file (or equivalent).
 * @param fileNameHatDevUT:     Name of the file that contains the
 *                              information needed to populate the hatDevUT
 *                              mapping. This should also be a plain text
 *                              .dta file (or equivalent).
 *
 */
TimeSVDPP::TimeSVDPP(int numUsers, int numItems, int numTimes,
                     float meanRating, int numFactors, int numIterations,
                     int numTimeBins,
                     const std::string &fileNameN,
                     const std::string &fileNameHatDevUT) :
    numUsers(numUsers), numItems(numItems), numTimes(numTimes),
    meanRating(meanRating), numFactors(numFactors),
    numIterations(numIterations), numTimeBins(numTimeBins),
    bUserConst(numUsers), bUserAlpha(numUsers),
    /* initialized later */ bUserTime(), bItemConst(numItems),
    bItemTimewise(numTimeBins, numItems), userFacMat(numFactors, numUsers),
    userFacMatAlpha(numFactors, numUsers), yMat(numFactors, numItems),
    itemFacMat(numFactors, numItems), numItemsTrainingSet(numUsers),
    sumMovieWeights(numFactors, numUsers)
{
    // Populate N by reading from fileNameN.
    populateN(fileNameN);

    // Populate hatDevUT by reading from fileNameHatDevUT
    populateHatDevUT(fileNameHatDevUT);

    // Initialize bUserConst, bUserAlpha, bItemConst, bItemTimewise,
    // userFacMat, userFacMatAlpha, itemFacMat, and yMat. The sparse
    // bUserTime matrix doesn't need initialization yet.
    initInternalData();

#ifndef NDEBUG
    cout << "Initialized data for Time-SVD++ predictor.\n" << endl;
#endif
}


/**
 * This constructor uses cached data to initialize the internals of the
 * TimeSVDPP object. It is assumed that all of the cached data (except the
 * file containing N) is stored in Armadillo's machine-dependent binary
 * format.
 *
 * @param numUsers:                 Number of users in the entire data set
 *                                  (not just training set).
 * @param numItems:                 Number of items in the entire data set
 *                                  (not just training set).
 * @param numTimes:                 Number of times (i.e. days) in the 
 *                                  entire data set (not just the training
 *                                  set).
 * @param meanRating:               The mean rating of items in the
 *                                  training set.
 * @param numFactors:               The number of factors to use for the
 *                                  matrix factorization.
 * @param numIterations:            The number of iterations to use for
 *                                  Time-SVD++.
 * @param numTimeBins:              The number of time bins to use for the
 *                                  time-dependent item bias.
 * @param fileNameN:                Name of the file that contains the
 *                                  information needed to populate the N
 *                                  mapping. This should be a plain text
 *                                  .dta file (or equivalent).
 * @param fileNameHatDevUT:         Name of the file that contains the
 *                                  information needed to populate the
 *                                  hatDevUT mapping. This should also be
 *                                  a plain text .dta file (or equivalent).
 * @param fileNameBUserConst:       Name of file containing data for
 *                                  bUserConst, in Armadillo's
 *                                  machine-dependent binary format.
 * @param fileNameBUserAlpha:       Same as above, but for bUserAlpha.
 * @param fileNameBUserTime:        Same as above, but for bUserTime.
 * @param fileNameBItemConst:       Same as above, but for bItemConst.
 * @param fileNameBItemTimewise:    Same as above, but for bItemTimewise.
 * @param fileNameUserFacMat:       Same as above, but for userFacMat.
 * @param fileNameUserFacMatAlpha:  Same as above, but for userFacMatAlpha.
 * @param fileNameItemFacMat:       Same as above, but for itemFacMat.
 * @param fileNameYMat:             Same as above, but for yMat.
 * @param fileNameSumMovieWeights:  Same as above, but for sumMovieWeights.
 *
 */
TimeSVDPP::TimeSVDPP(int numUsers, int numItems, int numTimes,
                     float meanRating, int numFactors, int numIterations,
                     int numTimeBins,
                     const std::string &fileNameN,
                     const std::string &fileNameHatDevUT,
                     const std::string &fileNameBUserConst,
                     const std::string &fileNameBUserAlpha,
                     const std::string &fileNameBUserTime,
                     const std::string &fileNameBItemConst,
                     const std::string &fileNameBItemTimewise,
                     const std::string &fileNameUserFacMat,
                     const std::string &fileNameUserFacMatAlpha,
                     const std::string &fileNameItemFacMat,
                     const std::string &fileNameYMat,
                     const std::string &fileNameSumMovieWeights) :
    numUsers(numUsers), numItems(numItems), numTimes(numTimes),
    meanRating(meanRating), numFactors(numFactors),
    numIterations(numIterations), numTimeBins(numTimeBins),
    bUserConst(numUsers), bUserAlpha(numUsers),
    bUserTime(numTimes, numUsers), bItemConst(numItems),
    bItemTimewise(numTimeBins, numItems), userFacMat(numFactors, numUsers),
    userFacMatAlpha(numFactors, numUsers), yMat(numFactors, numItems),
    itemFacMat(numFactors, numItems), numItemsTrainingSet() /* unused */,
    sumMovieWeights(numFactors, numUsers)
{
    // Populate N by reading from fileNameN.
    populateN(fileNameN);

    // Populate hatDevUT by reading from fileNameHatDevUT
    populateHatDevUT(fileNameHatDevUT);

    // Initialize bUserConst, bUserAlpha, bUserTime, bItemConst,
    // bItemTimewise, userFacMat, userFacMatAlpha, itemFacMat, yMat, and
    // sumMovieWeights by reading from their binary files.
    bUserConst.load(fileNameBUserConst, arma_binary);
    bUserAlpha.load(fileNameBUserAlpha, arma_binary);
    bUserTime.load(fileNameBUserTime, arma_binary);
    bItemConst.load(fileNameBItemConst, arma_binary);
    bItemTimewise.load(fileNameBItemTimewise, arma_binary);
    userFacMat.load(fileNameUserFacMat, arma_binary);
    userFacMatAlpha.load(fileNameUserFacMatAlpha, arma_binary);
    itemFacMat.load(fileNameItemFacMat, arma_binary);
    yMat.load(fileNameYMat, arma_binary);
    sumMovieWeights.load(fileNameSumMovieWeights, arma_binary);
    
    trained = true;
    usingCachedData = true;

#ifndef NDEBUG
    cout << "Created Time-SVD++ predictor using cached data." << endl;
#endif
}


/**
 * This function populates hatDevUT (the mapping from a user's ID and date
 * ID (as represented by a UserDate struct) to the hat{dev_u(t)} value for
 * that user and that date).
 *
 * @param fileNameHatDevUT: Name of the file that contains the information
 *                          needed to populate the hatDevUT mapping. This
 *                          should be a plain text .dta file (or
 *                          equivalent).
 *
 */
void TimeSVDPP::populateHatDevUT(const std::string &fileNameHatDevUT)
{
    std::ifstream fileHatDevUT(fileNameHatDevUT);

    if (fileHatDevUT.fail())
    {
        throw std::runtime_error("Couldn't find file containing "
                                 "hat{dev_u(t)} at " + fileNameHatDevUT);
    }

    std::string line;

    while (getline(fileHatDevUT, line))
    {
        std::istringstream lineSS(line);
        
        // Each line should be in the format <USER ID> <DATE ID>
        // <hat{dev_u(t)} FOR THAT RATING>
        int user;
        int date;
        float thisHatDevUT;
        
        lineSS >> user >> date >> thisHatDevUT;
        
        // Add on to the map hatDevUT.
        UserDate thisUserDate;
        thisUserDate.userID = user;
        thisUserDate.dateID = date;

        hatDevUT[thisUserDate] = thisHatDevUT;
    }

    fileHatDevUT.close();
}


/**
 * This function populates N (the mapping from zero-indexed user IDs to the
 * zero-indexed item IDs which that user has shown an implicit preference
 * for).
 *
 * @param fileNameN: Name of the file that contains the information needed
 *                   to populate the N mapping. This should be a plain text
 *                   .dta file (or equivalent).
 *
 */
void TimeSVDPP::populateN(const std::string &fileNameN)
{
    std::ifstream fileN(fileNameN);

    if (fileN.fail())
    {
        throw std::runtime_error("Couldn't find file containing N at " +
                                 fileNameN);
    }

    std::string line;

    while (getline(fileN, line))
    {
        // Split the string around the specified delimiter.
        std::vector<int> thisLineVec;
        splitIntoInts(line, DELIMITER, thisLineVec);
        
        // The first int should be the user's ID. This should be
        // zero-indexed!
        int userID = thisLineVec[0];

        // The remaining ints should be the item IDs that the user gave
        // "implicit feedback" on (without actually rating them). These
        // item IDs should be zero-indexed!
        std::vector<int> userImplFeedbackItems;
        
        for(std::vector<int>::size_type i = 1; i < thisLineVec.size(); i++)
        {
            userImplFeedbackItems.push_back(thisLineVec[i]);
        }
        
        // Add this data to the map N.
        N[userID] = userImplFeedbackItems;
    }

    fileN.close();
}


/**
 * Given a training set, this function updates numItemsTrainingSet -- an
 * array that stores the number of items in the training set that a given
 * user rated.
 *
 * @param data: This is the training data to use for our algorithm. See
 *              train() for more details.
 *
 */
void TimeSVDPP::populateNumItemsTrainingSet(const fmat &data)
{
    for(unsigned int i = 0; i < data.n_cols; i++)
    {
        // Based on the user that this rating was by, increment the
        // appropriate element of numItemsTrainingSet.
        int user = roundToInt(data(USER_ROW, i));
        
        numItemsTrainingSet(user) ++;
    }
    
}


/**
 *
 * This function initializes the internal data in this TimeSVDPP object.
 * Currently, randomization is turned on.
 *
 * The matrices to populate are: bUserConst, bUserAlpha, bItemConst,
 * bItemTimewise, userFacMat, userFacMatAlpha, itemFacMat, and yMat. The
 * sparse bUserTime matrix will be populated later (based on the training
 * set).
 *
 * Some of the initialization suggestions come from those for SVD++
 *  http://www.netflixprize.com/community/viewtopic.php?id=1359&p=2
 *
 * A lot of the time-related matrices are currently centered around zero in
 * their initialization.
 *
 */
void TimeSVDPP::initInternalData()
{
    // Different distributions based on the matrix being initialized.
    //std::uniform_real_distribution<float> distrBUserConst(-0.01, 0.1);
    //std::uniform_real_distribution<float> distrBUserAlpha(-0.02, 0.04);
    //std::uniform_real_distribution<float> distrBItemConst(-0.5, -0.1);
    //std::uniform_real_distribution<float> distrBItemTimewise(-0.03, 0.03);
    //std::uniform_real_distribution<float> distrUserFacMat(-0.018, -0.002);
    //std::uniform_real_distribution<float> distrUserFacMatAlpha(-0.005, 0.005);
    //std::uniform_real_distribution<float> distrItemFacMat(0.004, 0.02);
    //std::uniform_real_distribution<float> distrYMat(0.01, 0.02);
    
    std::uniform_real_distribution<float> coinFlip(-1.0, 1.0);

    // Set the seed to a sequence of random numbers that's large enough to
    // fill the mt19937's state.
    std::array<int, std::mt19937::state_size> seedData;
    std::random_device r;
    std::generate_n(seedData.data(), seedData.size(), std::ref(r));
    std::seed_seq seedSeq(begin(seedData), end(seedData));
    
    // Mersenne twister random number engine, based on the earlier seed.
    std::mt19937 engine(seedSeq);

    std::srand(std::time(NULL));
    
    //bUserConst.imbue( [&]() { return distrBUserConst(engine); } );
    //bUserAlpha.imbue( [&]() { return distrBUserAlpha(engine); } );
    //bItemConst.imbue( [&]() { return distrBItemConst(engine); } );
    //bItemTimewise.imbue( [&]() { return distrBItemTimewise(engine); } );
    userFacMat.imbue( [&]()
            {
                // From post #55 of 
                // http://www.netflixprize.com/community/viewtopic.php?id=1342&p=3
                // Note that copysign(1.0, coinFlip) gives a random sign
                // (either -1 or +1) with 50% probability of each case.
                return (std::rand() % 14000 + 2000) * 0.000001235 *
                       std::copysign(1.0, coinFlip(engine)); 
            });
    //userFacMatAlpha.imbue( [&]() { return distrUserFacMatAlpha(engine); });
    itemFacMat.imbue( [&]()
            {
                // From same source as above.
                return (std::rand() % 14000 + 2000) * 0.000001235 *
                       std::copysign(1.0, coinFlip(engine)); 
            });
    //itemFacMat.imbue( [&]() { return distrItemFacMat(engine); } );
    //yMat.imbue( [&]() { return distrYMat(engine); } );

    // Some matrices might be better off initialized to zero?
    bUserConst.zeros();
    bUserAlpha.zeros();
    userFacMatAlpha.zeros();
    bItemConst.zeros();
    bItemTimewise.zeros();
    yMat.zeros();

    // The sparse bUserTime matrix will be partly populated later.

    // This is the count of the number of items rated by users in the given
    // training set. We'll set this to zero for now.
    numItemsTrainingSet.zeros();
    
    // sumMovieWeights will be set up while training.
}


/** 
 * This function trains on a given set of data, and then caches the
 * internal data of this TimeSVDPP object.
 *
 * @param data:                     This is the training data to use for
 *                                  our algorithm. See train() for more
 *                                  details.
 * @param fileNameBUserConst:       Name of file containing data for
 *                                  bUserConst, in Armadillo's
 *                                  machine-dependent binary format.
 * @param fileNameBUserAlpha:       Same as above, but for bUserAlpha.
 * @param fileNameBUserTime:        Same as above, but for bUserTime.
 * @param fileNameBItemConst:       Same as above, but for bItemConst.
 * @param fileNameBItemTimewise:    Same as above, but for bItemTimewise.
 * @param fileNameUserFacMat:       Same as above, but for userFacMat.
 * @param fileNameUserFacMatAlpha:  Same as above, but for userFacMatAlpha.
 * @param fileNameItemFacMat:       Same as above, but for itemFacMat.
 * @param fileNameYMat:             Same as above, but for yMat.
 * @param fileNameSumMovieWeights:  Same as above, but for sumMovieWeights.
 * 
 */
void TimeSVDPP::trainAndCache(const fmat &data, 
                              const std::string &fileNameBUserConst,
                              const std::string &fileNameBUserAlpha,
                              const std::string &fileNameBUserTime,
                              const std::string &fileNameBItemConst,
                              const std::string &fileNameBItemTimewise,
                              const std::string &fileNameUserFacMat,
                              const std::string &fileNameUserFacMatAlpha,
                              const std::string &fileNameItemFacMat,
                              const std::string &fileNameYMat,
                              const std::string &fileNameSumMovieWeights)
{
    // Train the Time-SVD++ algorithm, then save internal data to file.
    train(data);

    // Save bUserConst, bUserAlpha, bUserTime, bItemConst, bItemTimewise,
    // userFacMat, userFacMatAlpha, itemFacMat, yMat, and
    // sumMovieWeights.
    bUserConst.save(fileNameBUserConst, arma_binary);
    bUserAlpha.save(fileNameBUserAlpha, arma_binary);
    bUserTime.save(fileNameBUserTime, arma_binary);
    bItemConst.save(fileNameBItemConst, arma_binary);
    bItemTimewise.save(fileNameBItemTimewise, arma_binary);
    userFacMat.save(fileNameUserFacMat, arma_binary);
    userFacMatAlpha.save(fileNameUserFacMatAlpha, arma_binary);
    itemFacMat.save(fileNameItemFacMat, arma_binary);
    yMat.save(fileNameYMat, arma_binary);
    sumMovieWeights.save(fileNameSumMovieWeights, arma_binary);
    
#ifndef NDEBUG
    cout << "Saved bUserConst to " << fileNameBUserConst << endl;
    cout << "Saved bUserAlpha to " << fileNameBUserAlpha << endl;
    cout << "Saved bUserTime to " << fileNameBUserTime << endl;
    cout << "Saved bItemConst to " << fileNameBItemConst << endl; 
    cout << "Saved bItemTimewise to " << fileNameBItemTimewise << endl;
    cout << "Saved userFacMat to " << fileNameUserFacMat << endl;
    cout << "Saved userFacMatAlpha to " << fileNameUserFacMatAlpha << endl;
    cout << "Saved itemFacMat to " << fileNameItemFacMat << endl;
    cout << "Saved yMat to " << fileNameYMat << endl;
    cout << "Saved sumMovieWeights to " << fileNameSumMovieWeights << endl;
#endif
}


/**
 * This function also trains and caches, but it first loads a file from
 * fileNameData. This file must be an Armadillo binary of an fmat.
 *
 * @param fileNameData: The file where "data" is stored. This binary file
 *                      must hold matrix data in the format specified in
 *                      the train(const fmat &data) function.
 *
 * The other params are the same as in the other trainAndCache()
 * function.
 *
 */
void TimeSVDPP::trainAndCache(const std::string &fileNameData,
                              const std::string &fileNameBUserConst,
                              const std::string &fileNameBUserAlpha,
                              const std::string &fileNameBUserTime,
                              const std::string &fileNameBItemConst, 
                              const std::string &fileNameBItemTimewise,
                              const std::string &fileNameUserFacMat,
                              const std::string &fileNameUserFacMatAlpha,
                              const std::string &fileNameItemFacMat,
                              const std::string &fileNameYMat,
                              const std::string &fileNameSumMovieWeights)
{
    fmat data;

    data.load(fileNameData, arma_binary);
    trainAndCache(data, fileNameBUserConst, fileNameBUserAlpha,
                  fileNameBUserTime, fileNameBItemConst,
                  fileNameBItemTimewise, fileNameUserFacMat,
                  fileNameUserFacMatAlpha, fileNameItemFacMat,
                  fileNameYMat, fileNameSumMovieWeights);
}




/**
 * This function updates the sum of movie weights (i.e. sum_{j in N(u)}
 * y_j) for each user u between lowUserNum (inclusive) and highUserNum
 * (exclusive). It is assumed that both of these user IDs are valid!
 *
 * @param lowUserNum:   The lower bound on user IDs to update.
 * @param highUserNum:  The upper bound (exclusive) on user IDs to update.
 *
 */
void TimeSVDPP::updateSumMovieWeights(int lowUserNum, int highUserNum)
{
    // Iterate over all users, get N[u], and compute.
    for(int user = lowUserNum; user < highUserNum; user++)
    {
        updateUserSumMovieWeights(user);
    }
}


/**
 * This function updates the sum of movie weights (i.e. sum_{j in N(u)}
 * y_j) for a single user.
 *
 * @param user:     The user ID of interest.
 *
 */
inline void TimeSVDPP::updateUserSumMovieWeights(int user)
{
    // Get N[u] and compute the desired sum.
    std::vector<int> nu = N[user];
    
    // Each column in sumMovieWeights has numFactors rows.
    fcolvec sumColVec = zeros<fcolvec>(numFactors);

    for (std::vector<int>::size_type ind = 0; ind < nu.size(); ind++)
    {
        int j = nu[ind];
        sumColVec += yMat.col(j);
    }

    sumMovieWeights.col(user) = sumColVec;
}


/**
 * This function uses the given training data in order to set up all of the
 * internal matrices needed for Time-SVD++. After training has been
 * completed, the "trained" boolean will be set to true.
 *
 * @param data: This is the training data to use for our algorithm. This
 *              must be a 4 x N matrix, where N is the total number of
 *              ratings in the training set. NOTE: The first column must
 *              contain user IDs, the second column must contain item IDs,
 *              the third column must contain date IDs, and the last column
 *              must contain the rating the user gave.  All of these are
 *              assumed to be floats.
 *
 * Precondition: "data" should be in column-major order as stated above.
 * Also, the users should be sorted by their user ID (i.e. no shuffling
 * should take place).
 *
 */

void TimeSVDPP::train(const fmat &data)
{
    // The predicted rating given by Time-SVD++ for user u and item i at
    // time t is:
    //      
    //      rHat_{ui}(t) = mu + b_u + alpha_{b_u} * hat{dev_u(t)} + b_{ut}
    //                     + b_i + b_{i, Bin(t)} + q_i^T * 
    //                     (p_u + alpha_{p_u} * hat{dev_u(t)} + 
    //                      |N(u)|^{-1/2} sum_{j in N(u)} y_j)
    //
    // Where we use a similar notation as in the BellKor paper (except
    // alpha terms are given more explicit subscripts). The goal of this
    // training procedure is to minimize the following function with
    // respect to q_*, p_*, y_*, and alpha_*:
    //
    // min sum_{(u, i, t) in K} ( (r_{ui}(t) - rHat_{ui}(t))^2 +
    //                          TIMESVDPP_LAM_B_U * b_u^2 + 
    //                          TIMESVDPP_LAM_ALPHA_B_U * alpha_{b_u}^2 +
    //                          TIMESVDPP_LAM_B_U_T * b_{ut}^2
    //                          TIMESVDPP_LAM_B_I_T * b_{i, Bin(t)}^2 +
    //                          TIMESVDPP_LAM_B_I * b_i^2 +
    //                          TIMESVDPP_LAM_Q_I * |q_i|^2 +
    //                          TIMESVDPP_LAM_P_U * |p_u|^2 +
    //                          TIMESVDPP_LAM_ALPHA_P_U * |alpha_{p_u}|^2 +
    //                          TIMESVDPP_LAM_Y_J * sum_{j in N(u)} |y_j|^2 )
    //
    // Where "K" is the training set and r_{ui}(t) is the actual rating
    // that the user gave for that item on that date. The regularization
    // terms here are used to prevent overfitting.
    //
    // This minimization is accomplished via stochastic gradient descent on
    // the free parameters.

    // Check that the data does in fact have four rows!
    if (data.n_rows != 4)
    {
        throw std::invalid_argument("Data array must have four rows!");
    }

    // If we're using cached data, we shouldn't be calling this method!
    if (usingCachedData)
    {
        throw std::logic_error("This algorithm shouldn't be trained if "
                               "you're using cached data!");
    }
        
    // If we've already trained on a previous dataset, we should reset all
    // of the internal data.
    if (trained)
    {
        initInternalData();
        
#ifndef NDEBUG
        cout << "Cleared old internal data" << endl;
#endif
    }
    
    // We want to find the number of items rated by each user in the
    // training set, since this will help us go through our training data
    // in a more organized fashion.
    populateNumItemsTrainingSet(data);
    
#ifndef NDEBUG
    time_point<system_clock> start, end;
    duration<float, std::ratio<60>> minutesElapsed; 
#endif

    // It's best if we add a small value epsilon to every entry in
    // bUserTime that we're actually going to use. This is more efficient
    // than initializing entries in the sparse matrix one at a time.

#ifndef NDEBUG
    // Start timing batch insertion.
    start = system_clock::now();
#endif
    
    // Small value to add to bUserTime.
    float epsilon = 0.0000001;

    umat locations(2, data.n_cols);
    fcolvec values(data.n_cols);

    // Keep track of previous user (assuming that the "data" matrix is
    // sorted by user IDs first).
    int prevUser = -1;
    
    // Keep track of date IDs for each user, to avoid repeats.
    std::unordered_set<int> dateIDsForThisUser;

    // The number of non-garbage entries in locations (and values).
    unsigned int numEntriesLocations = 0;

    for (unsigned int i = 0; i < data.n_cols; i++)
    {
        int user = roundToInt(data(USER_ROW, i));
        int date = roundToInt(data(DATE_ROW, i));

        if (user == prevUser)
        {
            // Check if we've seen this date ID for this user before. If
            // so, continue.
            if (dateIDsForThisUser.count(date) != 0)
            {
                continue;
            }
        }
        else
        {
            // The user changed; reinstantiate the set of date IDs.
            dateIDsForThisUser.clear();
        }
        
        // We want to insert an entry of "epsilon" in row "date" and column
        // "user" of bUserTime.
        locations(0, numEntriesLocations) = date;
        locations(1, numEntriesLocations) = user;
        values(numEntriesLocations) = epsilon;

        dateIDsForThisUser.insert(date);
        prevUser = user;

        numEntriesLocations++;
    }

    // Remove unused entries from "locations" and "values".
    locations.shed_cols(numEntriesLocations, data.n_cols - 1);
    values.shed_rows(numEntriesLocations, data.n_cols - 1);

    // Batch insertion constructor. The old matrix will be deallocated.
    bUserTime = sp_fmat(locations, values, numTimes, numUsers,
                        /* sort_locations */ true,
                        /* check_for_zeros */ false);

#ifndef NDEBUG
    end = system_clock::now();
    minutesElapsed = end - start;
    cout << "Set up sparse matrix bUserTime via batch insertion in "
        << minutesElapsed.count() << " minutes." << endl;
#endif


    
    // Iterate for the specified number of iterations.
    for(int iterCount = 0; iterCount < numIterations; iterCount++)
    {
#ifndef NDEBUG
        start = system_clock::now();
#endif
         
        // The rating number that we're looking at right now (i.e. the
        // column in our training set).
        unsigned int ratingNum = 0;
        
        // Iterate through all users in the training data. We're assuming
        // that the data is sorted (column-wise) by user ID!
        for (unsigned int user = 0; user < (unsigned int) numUsers;
             user ++)
        {
            // Update sumMovieWeights for this user.
            updateUserSumMovieWeights(user);

            // Check N(u) to see if this user has implicit feedback data.
            std::vector<int> nu = N[user];
            int nuSize = nu.size();
            
            if (nuSize == 0)
            {
                // If they don't, ignore them. After all, we're not gonna
                // predict anything for them anyways.
                continue;
            }
            
            // Norm factor put in front of userSumMovieWeights, etc.
            float nuNormFac = 1.0/sqrt((float) nuSize);
            
            // Find the number of items rated by this user in the training
            // set, so we know how many entries to parse.
            int numItemsUserTrainSet = numItemsTrainingSet[user];

            // The value of sum_{j in N(u)} y_j for this user.
            fcolvec userSumMovieWeights(sumMovieWeights.col(user));
            
            // The sum of all values of e_{ui} |N(u)|^{-1/2} * q_i over all
            // items watched by this user. This is used to update yMat via
            // gradient descent at the very end.
            fcolvec sumErrNuNormQi = zeros<fcolvec>(numFactors);

            // Construct a UserDate for this user's ratings. Use this to
            // find hat{dev_u(t)}. Note that dateID will be set in the loop
            // below.
            UserDate thisUserDate;
            thisUserDate.userID = user;
                        
            // Increment ratingNum as we iterate over items rated by the
            // user.
            for(int itemNum = 0; itemNum < numItemsUserTrainSet; itemNum++,
                                                                 ratingNum++)
            {
                int item = roundToInt(data(MOVIE_ROW, ratingNum));
                int date = roundToInt(data(DATE_ROW, ratingNum));
                float actualRating = data(RATING_ROW, ratingNum);
                
                // Update the UserDate struct.
                thisUserDate.dateID = date;
                
                // Item-wise time bins can range from 0 to numTimeBins. We
                // evenly divide (zero-indexed) dates into these bins.
                int timeBin = floor(date / NUM_DATES * numTimeBins);

                float thisHatDevUT = hatDevUT[thisUserDate];
                float oldBUserConst = bUserConst(user); 
                float oldBUserAlpha = bUserAlpha(user);
                float oldBUserTime = bUserTime(date, user);
                float oldBItemConst = bItemConst(item);
                float oldBItemTimewise = bItemTimewise(timeBin, item);
                
                fcolvec oldPU(userFacMat.col(user));
                fcolvec oldAlphaPU(userFacMatAlpha.col(user));
                fcolvec oldQI(itemFacMat.col(item));
                
                // Get the predicted rating for this user, item, and time,
                // using the aformentioned formula for rHat_{ui}(t).
                float predictedRating = meanRating + oldBUserConst +
                    oldBUserAlpha * thisHatDevUT + oldBUserTime + 
                    oldBItemConst + oldBItemTimewise;
                
                // Compute the factorized term (i.e. q_i^T * (p_u +
                // alpha_{p_u} * hat{dev_u(t)} + |N(u)|^{-1/2} sum_{j in
                // N(u)} y_j)).
                //
                // First find p_u + alpha_{p_u} * hat{dev_u(t)} +
                // |N(u)|^{-1/2} sum_{j in N(u)} y_j, the "userFactorTerm".
                // Start off by making a copy of p_u.
                fcolvec userFactorTerm(oldPU);

                // Add the time-dependent user factor bias.
                userFactorTerm += oldAlphaPU * thisHatDevUT;

                // sumMovieWeights should already have sum_{j in N(u)} y_j
                // cached (from the previous iteration), so use that old
                // value.
                userFactorTerm += userSumMovieWeights * nuNormFac;
                
                // Add the factorized term (q_i^T * userFactorTerm) to the
                // prediction.
                predictedRating += dot(oldQI, userFactorTerm);

                // Apply gradient descent on all of the free parameters in our
                // algorithm EXCEPT FOR yMat (which only needs to be updated at
                // the end for this user). This just involves subtracting off
                // the gradient of the error metric (which we're trying to
                // minimize) with respect to each free parameter. Note that
                // factors of 2 have been absorbed into the "gamma" step
                // sizes.
                
                // The error in our prediction for this user, item, and
                // time.
                float eUIT = actualRating - predictedRating;

                // b_u <- b_u + TIMESVDPP_GAMMA_B_U * (e_{uit} - 
                //          TIMESVDPP_LAM_B_U * b_u)
                bUserConst(user) += TIMESVDPP_GAMMA_B_U * (eUIT - 
                        TIMESVDPP_LAM_B_U * oldBUserConst);
                 
                // alpha_{b_u} <- alpha_{b_u} + TIMESVDPP_GAMMA_ALPHA_B_U *
                //                (e_{uit} * hat{dev_u(t)} - 
                //                 TIMESVDPP_LAM_ALPHA_B_U * alpha_{b_u})
                bUserAlpha(user) += TIMESVDPP_GAMMA_ALPHA_B_U * 
                    (eUIT * thisHatDevUT - TIMESVDPP_LAM_ALPHA_B_U * 
                                          oldBUserAlpha);

                // b_{ut} <- b_{ut} + TIMESVDPP_GAMMA_B_U_T * (e_{uit} -
                //          TIMESVDPP_LAM_B_U_T * b_{ut})
                bUserTime(date, user) += TIMESVDPP_GAMMA_B_U_T * (eUIT -
                        TIMESVDPP_LAM_B_U_T * oldBUserTime);
                 
                // b_i <- b_i + TIMESVDPP_GAMMA_B_I * (e_{uit} -
                //                      TIMESVDPP_LAM_B_I * b_i)
                bItemConst(item) += TIMESVDPP_GAMMA_B_I * (eUIT - 
                        TIMESVDPP_LAM_B_I * oldBItemConst);

                // b_{i, Bin(t)} <- b_{i, Bin(t)} + TIMESVDPP_GAMMA_B_I_T *
                //                  (e_{uit} - TIMESVDPP_LAM_B_I_T * 
                //                   b_{i, Bin(t)})
                bItemTimewise(timeBin, item) += TIMESVDPP_GAMMA_B_I_T *
                    (eUIT - TIMESVDPP_LAM_B_I_T * oldBItemTimewise);

                // q_i <- q_i + TIMESVDPP_GAMMA_Q_I * 
                //        (e_{uit} * (p_u + alpha_{p_u} * hat{dev_u(t)} +
                //         |N(u)|^{-1/2} sum_{j in N(u)} y_j) - 
                //         TIMESVDPP_LAM_Q_I * q_i)
                itemFacMat.col(item) += TIMESVDPP_GAMMA_Q_I * 
                    (eUIT * userFactorTerm - TIMESVDPP_LAM_Q_I * oldQI);
                
                // p_u <- p_u + TIMESVDPP_GAMMA_P_U * (e_{uit} * q_i -
                //          TIMESVDPP_LAM_P_U * p_u)
                userFacMat.col(user) += TIMESVDPP_GAMMA_P_U * (eUIT * oldQI
                        - TIMESVDPP_LAM_P_U * oldPU);
                
                // alpha_{p_u} <- alpha_{p_u} + TIMESVDPP_GAMMA_ALPHA_P_U *
                //          (e_{uit} * q_i * hat{dev_u(t)} -
                //           TIMESVDPP_LAM_ALPHA_P_U * alpha_{p_u})
                userFacMatAlpha.col(user) += TIMESVDPP_GAMMA_ALPHA_P_U *
                    (eUIT * oldQI * thisHatDevUT - 
                     TIMESVDPP_LAM_ALPHA_P_U * oldAlphaPU);
                
                // Update sumErrNuNormQi, which is the sum of all e_{uit}
                // |N(u)|^{-1/2} * q_i for this user (see SVD++ code).
                sumErrNuNormQi += eUIT * nuNormFac * oldQI;
                
            }
            
            // Go through each item in N[u] and update yMat for those
            // columns. Don't update sumMovieWeights for this user yet;
            // that'll happen on the next iteration.
            for (std::vector<int>::size_type ind = 0; ind < nu.size();
                 ind++)
            {
                // y_j <- y_j + TIMESVDPP_GAMMA_Y_J * (e_{ui} |N(u)|^{-1/2}
                //          * q_i - TIMESVDPP_LAM_Y_J * y_j)
                int j = nu[ind];
                fcolvec oldYJ = yMat.col(j);
                
                yMat.col(j) += TIMESVDPP_GAMMA_Y_J * (sumErrNuNormQi -
                        TIMESVDPP_LAM_Y_J * oldYJ);
            }

#if 0
            if (user % 10000 == 0)
            {
                cout << "Finished processing user #" << user << "." 
                     << endl;
            }
#endif
        }

        // At the end of each iteration, decrease the gammas by the
        // constant factor declared in the header file.
        TIMESVDPP_GAMMA_B_U *= TIMESVDPP_GAMMA_MULT_PER_ITER;
        TIMESVDPP_GAMMA_ALPHA_B_U *= TIMESVDPP_GAMMA_MULT_PER_ITER;
        TIMESVDPP_GAMMA_B_U_T *= TIMESVDPP_GAMMA_MULT_PER_ITER;
        TIMESVDPP_GAMMA_B_I *= TIMESVDPP_GAMMA_MULT_PER_ITER;
        TIMESVDPP_GAMMA_B_I_T *= TIMESVDPP_GAMMA_MULT_PER_ITER;
        TIMESVDPP_GAMMA_Q_I *= TIMESVDPP_GAMMA_MULT_PER_ITER;
        TIMESVDPP_GAMMA_P_U *= TIMESVDPP_GAMMA_MULT_PER_ITER;
        TIMESVDPP_GAMMA_ALPHA_P_U *= TIMESVDPP_GAMMA_MULT_PER_ITER;
        TIMESVDPP_GAMMA_Y_J *= TIMESVDPP_GAMMA_MULT_PER_ITER;
        
#ifndef NDEBUG
        end = system_clock::now();
        minutesElapsed = end - start;
        cout << "Finished iteration " << (iterCount + 1) << " of Time-"
             "SVD++ in " << minutesElapsed.count() << " minutes" 
             << endl;
#endif
    }

    // Update sumMovieWeights for the last time, so that the data used by
    // predict() (and the data cached to file) is accurate!
    updateSumMovieWeights(0, numUsers);

    trained = true;

#ifndef NDEBUG
    cout << endl;
#endif
}


/** 
 * This function predicts a rating for a given user, item, and date. If the
 * TimeSVDPP has not been trained yet, a logic_error is thrown.
 *
 * @param user: the user ID of interest.
 * @param item: the item ID of interest.
 * @param date: the date ID of interest.
 *
 * @return A prediction of the user's rating for the given item at the
 *         given time. This will always end up being between MIN_RATING and
 *         MAX_RATING.
 *
 * Precondition: It is assumed that sumMovieWeights has been accurately set
 * after training!
 *
 */
float TimeSVDPP::predict(int user, int item, int date)
{
    if (!trained)
    {
        throw std::logic_error("Tried to predict a rating but the Time-"
                               "SVD++ algorithm was not done training!");
    }

    // The predicted rating given by Time-SVD++ for user u and item i at
    // time t is:
    //      
    //      rHat_{ui}(t) = mu + b_u + alpha_{b_u} * hat{dev_u(t)} + b_{ut}
    //                     + b_i + b_{i, Bin(t)} + q_i^T * 
    //                     (p_u + alpha_{p_u} * hat{dev_u(t)} + 
    //                      |N(u)|^{-1/2} sum_{j in N(u)} y_j)
    
    // Construct a UserDate for this rating, and use this to find
    // hat{dev_u(t)}.
    UserDate thisUserDate;
    thisUserDate.userID = user;
    thisUserDate.dateID = date;
    
    std::vector<int> nu = N[user];
    float nuNormFac = 1.0/sqrt(nu.size());
    float thisHatDevUT = hatDevUT[thisUserDate];
    
    // Item-wise time bins can range from 0 to numTimeBins. We evenly
    // divide (zero-indexed) dates into these bins.
    int timeBin = floor(date / NUM_DATES * numTimeBins);

    // Get the predicted rating for this user, item, and time, using the
    // aformentioned formula for rHat_{ui}(t).
    float predictedRating = meanRating + bUserConst(user) +
        bUserAlpha(user) * thisHatDevUT + bUserTime(date, user) + 
        bItemConst(item) + bItemTimewise(timeBin, item);
    
    // Compute the factorized term (i.e. q_i^T * (p_u + alpha_{p_u} *
    // hat{dev_u(t)} + |N(u)|^{-1/2} sum_{j in N(u)} y_j)).
    //
    // First find p_u + alpha_{p_u} * hat{dev_u(t)} + |N(u)|^{-1/2} sum_{j
    // in N(u)} y_j, the "userFactorTerm".  Start off by making a copy of
    // p_u.
    fcolvec userFactorTerm(userFacMat.col(user));

    // Add the time-dependent user factor bias.
    userFactorTerm += userFacMatAlpha.col(user) * thisHatDevUT;

    // Get sum_{j in N(u)} y_j and multiply by nuNormFac.
    userFactorTerm += sumMovieWeights.col(user) * nuNormFac;
    
    // Add the factorized term (q_i^T * userFactorTerm) to the prediction.
    fcolvec qi(itemFacMat.col(item));
    predictedRating += dot(qi, userFactorTerm);
    
    // Put the rating between MIN_RATING and MAX_RATING! Otherwise, the
    // error will be bad.
    if (predictedRating < MIN_RATING)
    {
        predictedRating = (float) MIN_RATING;
    }
    else if (predictedRating > MAX_RATING)
    {
        predictedRating = (float) MAX_RATING;
    }
    
    return predictedRating;
}


TimeSVDPP::~TimeSVDPP()
{
    // No dynamically allocated resources to free at the moment.
}
