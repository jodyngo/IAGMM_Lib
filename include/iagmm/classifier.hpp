#ifndef CLASSIFIER_HPP
#define CLASSIFIER_HPP

#include <functional>
#include <boost/shared_ptr.hpp>
#include <eigen3/Eigen/Core>
#include <tbb/tbb.h>
#include "data.hpp"

namespace iagmm {

/**
 * @brief The Classifier class
 * Mother class of all classifier. this is an abstract class.
 */
class Classifier{
public:

    typedef boost::shared_ptr<Classifier> Ptr;
    typedef boost::shared_ptr<const Classifier> ConstPtr;


    typedef std::function<double(const Eigen::VectorXd&,const Eigen::VectorXd&)> _distance_f;


    /**
     * @brief default Constructor
     */
    Classifier(){}

    /**
     * @brief basic constructor. Specify the dimension of the features space and the number of class
     * @param dimension of the feature space
     * @param nbr_class number of class
     */
    Classifier(int dimension, int nbr_class = 2) :
        _dimension(dimension), _nbr_class(nbr_class){}

    /**
     * @brief copy constructor
     * @param c a classifier.
     */
    Classifier(const Classifier &c) :
        _nbr_class(c._nbr_class),
        _dimension(c._dimension),
        _samples(c._samples),
        _distance(c._distance){}

    virtual ~Classifier(){}

    /**
     * @brief compute_estimation : Compute a probability for a "sample" to be part of the class "lbl"
     * @param sample
     * @param label of the class
     * @return the probability of the sample to be part of the class lbl
     */
    virtual std::vector<double> compute_estimation (const Eigen::VectorXd& sample) = 0;

    /**
     * @brief update the classifier according the dataset
     */
    virtual void update() = 0;

    /**
     * @brief compute the classication confidence of sample
     * @param a sample
     * @return a value between 1 and 0
     */
    virtual double confidence(const Eigen::VectorXd& sample) const = 0;

    /**
     * @brief compute a choice distribution map over the input samples which give the probability do be chosen
     * @param list of samples with their classification probability
     * @param choice distribution map
     * @return the index of the next sample to add in the dataset from the list samples
     */
    virtual int next_sample(const std::vector<std::pair<Eigen::VectorXd,std::vector<double>>>& samples,Eigen::VectorXd& choice_dist_map) = 0;


    /**
     * @brief predict the label of the training dataset
     * @param training dataset
     * @param results the prediction of label
     * @return error of prediction
     */
    virtual double predict(const TrainingData& data, std::vector<std::vector<double>>& results){
        results.resize(data.size());
        tbb::parallel_for(tbb::blocked_range<size_t>(0,data.size()),
                          [&](const tbb::blocked_range<size_t>& r){
            for(int i = r.begin(); i != r.end(); i++)
                results[i] = compute_estimation(data[i].second);
        });
        double error = 0;
        for(int i = 0; i < data.size(); i++){
            error = error + 1 - results[i][data[i].first];
        }
        return error/(double)data.size();
    }

    /**
     * @brief add a sample to the training set of the classifier
     * @param sample
     * @param the label of the classifier
     */
    virtual void add(const Eigen::VectorXd& sample, int lbl){
        _samples.add(lbl,sample);
    }

    /**
     * @brief dataset_size
     * @return size of the training dataset
     */
    size_t dataset_size() const {return _samples.size();}

    /**
     * @brief get_samples
     * @return all the samples of the training dataset
     */
    const TrainingData& get_samples() const {return _samples;}
    void set_samples(TrainingData samples){_samples = samples;}
    int get_nbr_class() const {return _nbr_class;}
    int get_dimension() const {return _dimension;}

    void set_distance_function(_distance_f distance){
        _distance = distance;
    }

    void _estimate_training_dataset(){
        _samples.estimations.resize(_samples.size());
        tbb::parallel_for(tbb::blocked_range<size_t>(0,_samples.size()),
                          [&](const tbb::blocked_range<size_t>& r){
            std::vector<double> estimates(_nbr_class);
            for(int i = r.begin(); i != r.end(); i++){
                estimates = compute_estimation(_samples[i].second);
                _samples.estimations[i] = estimates;
            }
        });
    }

protected:
    int _nbr_class;
    int _dimension;
    TrainingData _samples;

    _distance_f _distance;
};

}
#endif //CLASSIFIER_HPP
