#ifndef CATS2_DIJKSTRA_PATH_PLANNER_HPP
#define CATS2_DIJKSTRA_PATH_PLANNER_HPP

#include "SetupMap.hpp"
#include "GridBasedMethod.hpp"

#include <AgentState.hpp>

#include <QtCore/QPoint>
#include <QtCore/QMap>

#include <boost/config.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/property_map/property_map.hpp>

bool operator<(QPoint const& p1, QPoint const& p2);

/*!
 * Runs the path planner to safety reach the target position by using the
 * Dijkstra path planner.
 */
class DijkstraPathPlanner : public GridBasedMethod
{
public:
    //! Constructor.
    DijkstraPathPlanner();
    //! Destructor.
    ~DijkstraPathPlanner();

    //! Generates a path plan from the current to the target position.
    QQueue<PositionMeters> plan(PositionMeters start, PositionMeters goal);

public:
    //! Returns the validity flag.
    bool isValid() const { return m_valid; }

private:
    //! Builds the graph.
    bool init();

    //! Adds an edge between two grid nodes.
    void addEdge(QPoint startNode, QPoint endNode);

    //! Simplifies the resulted path by removing the points lying on the same
    //! line.
    void simplifyPath(QQueue<PositionMeters>& path);

private:
    struct vertexInfo {
        int row; // up
        int col; // left to right
    };

    //! The graph class.
    //! https://theboostcpplibraries.com/boost.graph-algorithms
    //! By passing boost::listS as the first template parameter, std::list is
    //! selected as the container for lines. The second template parameter tells
    //!  which class should be used for points. boost::vecS is the default value.
    //! The third template parameter determines whether lines are directed or
    //! undirected. The fourth and fifth template parameter specify if points and
    //! lines have properties and what those properties are. The fifth parameter
    //! uses boost::property to define a bundled property. Bundled properties are
    //! properties that are stored internally in a graph. Because it’s possible to
    //! define multiple bundled properties, boost::property expects a tag to define
    //! each property. Boost.Graph provides some tags, such as boost::edge_weight_t,
    //! to define frequently used properties that are automatically recognized
    //! and used by algorithms. The second template parameter passed to boost::
    //! property is the type of the property.
    using UndirectedGraph = boost::adjacency_list
                                <boost::vecS, boost::vecS,
                                 boost::undirectedS, vertexInfo,
                                 boost::property<boost::edge_weight_t, double >>;
    //! The vertex class.
    using Vertex = boost::graph_traits<UndirectedGraph>::vertex_descriptor;

private:
    //! A flag that says if the path planner was correctly initialized.
    bool m_valid;
    //! The graph representing the setup.
    UndirectedGraph m_graph;

    //! Maps the grid nodes' coordinates to the vertices descriptors in the graph.
    QMap <QPoint, Vertex>  m_gridNodeToVertexMap;
    //! The vector that contains to which component belongs every vertex.
    std::vector<Vertex> m_componentByVertex;

    //! Defines the maximal distance between two points in the path. It's
    //! introduced to prevent long lines between the intermediate points that
    //! would make the robot to bump into walls in the setups with corridors
    //! before entering to a corridor.
    static constexpr double MaximalDistanceBetweenTwoPathPoints = 0.10;

    //! A flag to limit the number of error messages.
    bool m_gotErrorOnPreviousStep;
};

#endif // CATS2_DIJKSTRA_PATH_PLANNER_HPP
