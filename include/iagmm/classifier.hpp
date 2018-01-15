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
     * @brief compute_estimation : Compute a probability for a "sample" to be of the class "lbl"
     * @param sample
     * @param label of the class
     * @return the probability of the sample to be part of the class lbl
     */
    virtual double compute_estimation (const Eigen::VectorXd& sample, int lbl) = 0;
    virtual void update() = 0;
    virtual double confidence(const Eigen::VectorXd& sample) const = 0;
    virtual int next_sample(const std::vector<std::pair<Eigen::VectorXd,double>>& samles,Eigen::VectorXd& choice_dist_map) = 0;



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

protected:
    void _estimate_training_dataset(){
        _samples.estimations.resize(_samples.size());
        tbb::parallel_for(tbb::blocked_range<size_t>(0,_samples.size()),
                          [&](const tbb::blocked_range<size_t>& r){
            for(int i = r.begin(); i != r.end(); i++)
                _samples.estimations[i] = compute_estimation(_samples[i].second,_samples[i].first);
        });
    }

    int _nbr_class;
    int _dimension;
    TrainingData _samples;

    _distance_f _distance;
};

}
#endif //CLASSIFIER_HPP
