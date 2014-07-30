#ifndef OBSTACLETRACKER_H
#define OBSTACLETRACKER_H

/// STL
#include <vector>

/// THIRD PARTY
#include <opencv2/imgproc/imgproc.hpp>

/// ROS
#include <ros/ros.h>

/// PROJECT
#include "maptransformer.h"

/**
 * @brief Track obstacles on the path.
 *
 * The tracking algorithm (implemented in update()) is quite simple:
 * Input: New observation (= list of detected obstacles)
 *   - Try to match observed obstacles with those that are already tracked (see below, how matching is done).
 *   - Drop tracked obstacles which had no match for a defined duration.
 *   - Add observed obstacles w/o matching partner in the list of currently tracked obstacles as new.
 *
 * Matching:
 *   1) Construct a matrix with all distances between tracked and observed obstacles (observed = rows, tracked = columns)
 *   2) Search the matrix for the pair with the minimal distance. Match this pair.
 *   3) Remove the row and the column, related to the matched obstacles, from the distance matrix.
 *   4) Iterate until distance matrix is empty (= all are matched), or the minimal distance in the remaining matrix is
 *      greater than a defined threshold (-> do not match, if they are to far away from each other).
 *
 * Currently only the center point of an obstacle is used.
 */
class ObstacleTracker
{
public:
    class TrackedObstacle {
    public:
        TrackedObstacle(cv::Point2f pos):
            last_position_(pos),
            time_of_first_sight_(ros::Time::now()),
            time_of_last_sight_(ros::Time::now())
        {}

        void update(cv::Point2f pos)
        {
            last_position_ = pos;
            time_of_last_sight_ = ros::Time::now();
        }

        cv::Point2f last_position() const
        {
            return last_position_;
        }

        ros::Time time_of_first_sight() const
        {
            return time_of_first_sight_;
        }

        ros::Time time_of_last_sight() const
        {
            return time_of_last_sight_;
        }

    private:
        //std::vector<cv::Point> contour;
        //! Last known position of the obstacle.
        cv::Point2f last_position_;
        //! Time, when the obstacle was detected for the first time (i.e. when it has been added to the list of tracked obstacles).
        ros::Time time_of_first_sight_;
        //! Time, when the obstacle was detected for the last time (to track lost obstacles).
        ros::Time time_of_last_sight_;
    };

    ObstacleTracker():
        max_dist_(0.3f),
        lost_lifetime_(1) //FIXME: parameters
    {}

    void setMaxDist(float md)
    {
        max_dist_ = md;
    }

    std::vector<TrackedObstacle> getTrackedObstacles() const
    {
        return obstacles_;
    }


    /**
     * @brief Update the list ob tracked obstacles with a new observation.
     * @param obstacles List of obstacles observed in the last scan.
     */
    void update(std::vector<cv::Point2f> obstacles);

private:
    //! Only match new observation with tracked obstacle, if the position change is less than this threshold.
    float max_dist_;

    //! Duration for which a lost obstacle is still tracked at its last known position.
    ros::Duration lost_lifetime_;

    std::vector<TrackedObstacle> obstacles_;

    bool isDead(TrackedObstacle o);

    /**
     * @brief Remove a row from a matrix.
     * @param m    The matrix.
     * @param row  Row-Index.
     */
    static inline void removeRow(cv::Mat &m, int row);

    /**
     * @brief Remove a column from a matrix.
     * @param m    The matrix.
     * @param col  Column-Index.
     */
    static inline void removeColumn(cv::Mat &m, int col);
};

#endif // OBSTACLETRACKER_H
