#include "ukf/predictor.hpp"

class future_trajectory_predictor
{

public:
    future_trajectory_predictor(int num_future_sec) {
        this->init(num_future_sec);
    }
    ~future_trajectory_predictor() {
    }

private:
    void init(int num_future_sec)
    {
        ukf_predictor_ptr_.reset(new ukf_predictor);
        num_future_sec_ = num_future_sec;
    }

public:
    void setFutureTrajPredParam(int num_state, int num_state_noise, int num_meas,
                                float alpha, float beta, float lamda){

        int num_aug = num_state + num_state_noise + num_meas;
        int num_sigma_points = 2 * (num_aug) + 1;
        Eigen::VectorXf weight_m; weight_m.resize(num_sigma_points);
        Eigen::VectorXf weight_c; weight_c.resize(num_sigma_points);

        weight_m(0) = lamda / (lamda + num_aug);
        weight_c(0) = lamda / (lamda + num_aug) + (1 - pow(alpha,2) +  beta);

        for(int i=1; i < num_sigma_points; ++i)
        {
            weight_m(i) = 0.5 / (lamda + num_aug);
            weight_c(i) = 0.5 / (lamda + num_aug);
        }

        ukf_predictor_ptr_->setPredictionParams(num_state, num_meas, num_aug, num_state_noise,
                                                num_sigma_points, lamda,
                                                weight_m, weight_c);
    }

    std::vector<Eigen::VectorXf> generateFutureTrajectory(Eigen::VectorXf current_state_x,
                                                          Eigen::MatrixXf current_state_p,
                                                          Eigen::MatrixXf Q, Eigen::MatrixXf R, float dt){

        std::vector<Eigen::VectorXf> future_state_vec;
        dt = 0.1;
        for(int i =0; i < num_future_sec_; ++i)
        {
            Eigen::MatrixXf Xsig_aug;
            //keeping state cov P as the curr_state_P at each iteration
            Eigen::MatrixXf const_p; const_p = current_state_p;
            Xsig_aug = ukf_predictor_ptr_->generateSigmaPoints(current_state_x, const_p,
                                                               Q, R);
            Eigen::MatrixXf X_predicted;
            X_predicted = ukf_predictor_ptr_->predictUsingSigmaPoints(Xsig_aug, dt);

            current_state_x = ukf_predictor_ptr_->predictMeanAndCovariance(X_predicted, const_p);

            future_state_vec.push_back(current_state_x);

        }

        return future_state_vec;
    }

private:
    int num_future_sec_;
    std::unique_ptr<ukf_predictor> ukf_predictor_ptr_;
};
