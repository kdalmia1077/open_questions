#pragma once 

#include "utils.hpp"

namespace AlgoComp {

  class InputChangeListener {
  public:
    virtual ~InputChangeListener ( ) { };

    virtual double OnInputChange ( unsigned int input_index_, double input_value_ ) = 0;
  };


  class OutputChangeListener {
  public:
    virtual ~OutputChangeListener ( ) { };

    virtual void OnOutputChange ( double _new_out_value_ ) = 0;
  };


  class TertiaryRandomForest : public InputChangeListener {
  private:
    std::vector < OutputChangeListener * > output_change_listeners_;
    Forest* forest_;

    std::vector < double > predictor_values_;
    double previous_output_;

    double ComputeOutputForForest ( ) {
      double total = 0;
      unsigned int forest_size = forest_->tree_vec_.size();           
      for ( unsigned int i = 0u; i < forest_size; i++ ) {
        total += ComputeOutputForTree ( &( forest_->tree_vec_[i] ) );
      }

      return total / forest_size ;
    }

    double ComputeOutputForTree (Tree * tree) {
      // Check if tree is empty
      if ( tree->size() == 0 ) {
        return 0;
      }

      // Use the current predictor_values_ to compute output
      return GetValueFromNode (tree, 0);
    }

    double GetValueFromNode ( Tree * tree, int node_index ) {
      // Create a reference to the object
      Node& node = (*tree)[node_index];
      if ( node.is_leaf_ ) {
        return node.predicted_value_;
      }

      double predictor_value = predictor_values_[node.predictor_index_];
      double low = node.boundary_value_vec_[0];
      double high = node.boundary_value_vec_[1];

      if ( predictor_value < low ) {
        return GetValueFromNode ( tree, node.child_node_index_vec_[0] );
      } else if (predictor_value > high ) {
        return GetValueFromNode ( tree, node.child_node_index_vec_[2] );
      } else {
        return GetValueFromNode ( tree, node.child_node_index_vec_[1] );
      }
    }

  public:
    TertiaryRandomForest ( ) { }
    
    //Will be called for initializing the Forest object
    //Can use Forest Class in utils.hpp to parse the forestfile
    void InitializeForest ( const char * const forest_filename_ ) {
      forest_ = new Forest ( forest_filename_ );

      //std::cout << "Forest has been Initialized" << std::endl;
      //std::cout << "Number of Trees = " << forest_->tree_vec_.size() << std::endl;
      //std::cout << "Number of Predictors = " << forest_->num_predictors_ << std::endl;

      // Initialize the predictors vector
      for ( unsigned int i = 0u; i < forest_->num_predictors_; i++ ) {
        predictor_values_.push_back( 0 );
      }

      // Compute initial value
      previous_output_ = ComputeOutputForForest( );
    }

    //Will be called to notfiy changes in predictor values
    //Should return the updated output value
    double OnInputChange ( unsigned int input_index_, double input_value_ ) { 
      // Update the predictors vector
      predictor_values_[input_index_] = input_value_;

      // Compute the new output
      double output = ComputeOutputForForest( );

      // Notify all listeners
      for ( unsigned int i = 0u; i < output_change_listeners_.size(); i++ ) {
        ( output_change_listeners_[i] )->OnOutputChange( output );
      }
      return 0; 
    }
    
    //_new_listener_ shoud be notified by calling _new_listener_->OnOutputChange on every change in Forest Output
    //should return true if _new_listener_ is successfully subscribed to Forest output updates
    bool SubscribeOutputChange ( OutputChangeListener * _new_listener_ ) { 
      output_change_listeners_.push_back( _new_listener_ );
      return true; 
    }

  };

}
