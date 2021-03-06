#include "stdafx.h"

#include "RandomForest.h"
#include "ParallelRandomForest.h"

using namespace std;

typedef boost::mt19937 RNGType;

typedef boost::variate_generator<RNGType, boost::uniform_int<> > RNG;

const void RandomForest::serialize(vector<uint8_t>& v) const
{
   v.clear();
   BOOST_FOREACH(const DecisionTree& tree, _forest)
   {
      tree.serialize(v);
      //res.insert(res.cend(), treeBin.begin(), treeBin.end());
   }
   //return res;
}

void RandomForest::deserialize(vector<uint8_t>& v)
{
   vector<uint8_t>::iterator itr = v.begin();
   clear();
   while (itr != v.end())
   {
      _forest.push_back(DecisionTree());
      _forest[_forest.size() - 1].deserialize(itr);
   }
}

void trainRandomForest(RandomForest& randomForest, const vector<Item>& trainData, const vector<Item>& testData, uint32_t seed, int mink, int maxk) //because 42 is answer to life the universe and everything!
{
   vector<const Item*> ptrTrainData;
   vector<const Item*> ptrTestData;
   for (int i = 0; i < trainData.size(); ++i)
   {
      ptrTrainData.push_back(&trainData[i]);
   }
   for (int i = 0; i < testData.size(); ++i)
   {
      ptrTestData.push_back(&testData[i]);
   }
   trainRandomForest(randomForest, ptrTrainData, ptrTestData, seed, mink, maxk);
}

TrainData* VectorTreesIterator::operator()(tbb::flow_control& fc) const
{
   if (_itr == _end)
   {
      fc.stop();
      return 0;
   }
   cout << (_end - _itr) << endl;
   TrainData* ret = new TrainData(*_itr, *_itrSubsets, *_itrSubfeatures);
   ++_itr;
   ++_itrSubsets;
   ++_itrSubfeatures;
   return ret;//���������� ���������(DecisionTree*) �� ��������, ������� ��������, ����������� ��������. ���������� ���������� ���������, � �������� ������� � ���������� �������� ���������.
}

void TrainDecisionTree::operator()(TrainData* ptr) const
{
   trainDecisionTree(*(ptr->_tree), *(ptr->_trainData), *(ptr->_numberOfUsedFeatures));
   delete ptr;
}

void trainRandomForest(RandomForest& randomForest, const vector<const Item*>& trainData, const vector<const Item*>& testData, uint32_t seed, int mink, int maxk) //because 42 is answer to life the universe and everything!
{
   //validate train data
   if (trainData.empty())
      return;
   //initialize params
   randomForest.clear();
   if (mink == 0)
   {
      maxk = trainData.size();
   }
   else
   {
      --mink;
      --maxk;
   }
   int M = trainData[0]->_featuresSize;
   int m = (int)floor(sqrt((double)M));
   //generate subsets
   vector<vector<const Item*> > subset(maxk,vector<const Item*>());
   vector<vector<int> > subfeature(maxk,vector<int>());
   RNG rngN(RNGType(seed), boost::uniform_int<>(0, trainData.size() - 1));
   RNG rngB(RNGType(seed), boost::uniform_int<>(0, INT32_MAX));
   for (int k = 0; k < maxk; ++k)
   {
      for (int j = 0; j < trainData.size(); ++j)
         subset[k].push_back(trainData[rngN()]);
      for (int j = 0, t = m; j < M; ++j)
      {
         if ((rngB() % (M - j)) < t)
         {
            subfeature[k].push_back(j);
            --t;
         }
      }
   }
   //create trees and calc tests predict
   randomForest._forest.resize(maxk);
    /*for (int k = 0; k <= mink; ++k)
   {
      cout << k << "/" << maxk << endl;
      trainDecisionTree(randomForest._forest[k], subset[k], subfeature[k]);
      DecisionTree &tree = randomForest._forest[k];
      for (int i = 0; i < testData.size(); ++i)
      {
         tests[i][0] += tree.getClassTrueProbability(*testData[i]);
      }
   }
   for (int k = mink + 1; k < maxk; ++k)
   {
      cout << k << "/" << maxk << endl;
      trainDecisionTree(randomForest._forest[k], subset[k], subfeature[k]);
      DecisionTree &tree = randomForest._forest[k];
      for (int i = 0; i < testData.size(); ++i)
      {
         tests[i].push_back(tree.getClassTrueProbability(*testData[i]));
         tests[i][k - mink] += tests[i][k - 1 - mink];
      }
   }*/
   vector <vector<double> > tests(testData.size(), vector<double>(1,0));
   tbb::filter_t<void, TrainData*> iterator = tbb::make_filter<void, TrainData*>(tbb::filter::serial_in_order, VectorTreesIterator(randomForest._forest, subset, subfeature));
   tbb::filter_t<TrainData*, void> calculator = tbb::make_filter<TrainData*, void>(tbb::filter::parallel, TrainDecisionTree());
   tbb::filter_t <void, void> trainProcess = iterator & calculator;
   parallel_pipeline(tbb::task_scheduler_init::default_num_threads(), trainProcess);
   cout << "Success train!\n";
   for (int k = 0; k <= mink; ++k)
   {
      DecisionTree &tree = randomForest._forest[k];
      for (int i = 0; i < testData.size(); ++i)
      {
         tests[i][0] += tree.getClassTrueProbability(*testData[i]);
      }
   }
   for (int k = mink + 1; k < maxk; ++k)
   {
      DecisionTree &tree = randomForest._forest[k];
      for (int i = 0; i < testData.size(); ++i)
      {
         tests[i].push_back(tree.getClassTrueProbability(*testData[i]));
         tests[i][k - mink] += tests[i][k - 1 - mink];
      }
   }
   //calculate mean quadratic error
   vector <double> meanQuadraticError(maxk - mink, 0);
   for (int k = mink; k < maxk; ++k)
   {
      for (int i = 0; i < testData.size(); ++i)
      {
         double x = tests[i][k - mink] / (k + 1);
         if (testData[i]->_class)
            x = (1.0 - x) * (1.0 - x);
         else
            x = x * x;
         meanQuadraticError[k - mink] += x;
      }
      meanQuadraticError[k - mink] /= testData.size();
   }
   //find minimum
   int kForMinimumMQE = mink;
   for (int k = mink + 1; k < maxk; ++k)
   {
      if (meanQuadraticError[k - mink] < meanQuadraticError[kForMinimumMQE])
         kForMinimumMQE = k;
   }
   randomForest._forest.resize(kForMinimumMQE + 1);
   //Congratulations!!!
}