#ifndef __NABO_H
#define __NABO_H

#include "Eigen/Core"
#include "Eigen/Array"
#include <vector>

namespace Nabo
{
	// Euclidean distance
	template<typename T>
	T dist2(const typename Eigen::Matrix<T, Eigen::Dynamic, 1> v0, const typename Eigen::Matrix<T, Eigen::Dynamic, 1> v1)
	{
		return (v0 - v1).squaredNorm();
	}

	// Nearest neighbor search interface, templatized on scalar type
	template<typename T>
	struct NearestNeighborSearch
	{
		typedef typename Eigen::Matrix<T, Eigen::Dynamic, 1> Vector;
		typedef typename Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> Matrix; // each entry is a col, matrix has dim rows
		typedef int Index;
		typedef std::vector<Index> Indexes;
		
		const Matrix& cloud;
		const size_t dim;
		const Vector minBound;
		const Vector maxBound;
		
		struct Statistics
		{
			Statistics(): lastQueryVisitCount(0), totalVisitCount(0) {}
			int lastQueryVisitCount;
			int totalVisitCount;
		};
		
		NearestNeighborSearch(const Matrix& cloud);
		virtual Indexes knn(const Vector& query, const Index k = 1, const bool allowSelfMatch = false) = 0;
		const Statistics getStatistics() const { return statistics; }
		
	protected:
		Statistics statistics;
	};

	// Brute-force nearest neighbor
	template<typename T>
	struct BruteForceSearch: public NearestNeighborSearch<T>
	{
		typedef typename NearestNeighborSearch<T>::Vector Vector;
		typedef typename NearestNeighborSearch<T>::Matrix Matrix;
		typedef typename NearestNeighborSearch<T>::Index Index;
		typedef typename NearestNeighborSearch<T>::Indexes Indexes;

		BruteForceSearch(const Matrix& cloud);
		virtual Indexes knn(const Vector& query, const Index k, const bool allowSelfMatch);
	};
	
	template<typename T>
	struct KDTree: public NearestNeighborSearch<T>
	{
		typedef typename NearestNeighborSearch<T>::Vector Vector;
		typedef typename NearestNeighborSearch<T>::Matrix Matrix;
		typedef typename NearestNeighborSearch<T>::Index Index;
		typedef typename NearestNeighborSearch<T>::Indexes Indexes;
		
	protected:
		struct BuildPoint
		{
			Vector pos;
			size_t index;
			BuildPoint(const Vector& pos =  Vector(), const size_t index = 0): pos(pos), index(index) {}
		};
		typedef std::vector<BuildPoint> BuildPoints;
		typedef typename BuildPoints::iterator BuildPointsIt;
		typedef typename BuildPoints::const_iterator BuildPointsCstIt;
		
		struct CompareDim
		{
			size_t dim;
			CompareDim(const size_t dim):dim(dim){}
			bool operator() (const BuildPoint& p0, const BuildPoint& p1) { return p0.pos(dim) < p1.pos(dim); }
		};
		
		struct SearchElement
		{
			size_t index;
			T minDist;
			
			SearchElement(const size_t index, const T minDist): index(index), minDist(minDist) {}
			// invert test as std::priority_queue shows biggest element at top
			friend bool operator<(const SearchElement& e0, const SearchElement& e1) { return e0.minDist > e1.minDist; }
		};
		
		struct Node
		{
			Vector pos;
			int dim; // -1 == leaf, -2 == invalid
			Index index;
			Node(const Vector& pos = Vector(), const int dim = -2, const Index index = 0):pos(pos), dim(dim), index(index) {}
		};
		typedef std::vector<Node> Nodes;
		
		Nodes nodes;
		
		inline size_t childLeft(size_t pos) const { return 2*pos + 1; }
		inline size_t childRight(size_t pos) const { return 2*pos + 2; }
		inline size_t parent(size_t pos) const { return (pos-1)/2; }
		size_t getTreeSize(size_t size) const;
		size_t argMax(const Vector& v) const;
		Indexes cloudIndexesFromNodesIndexes(const Indexes& indexes) const;
		void buildNodes(const BuildPointsIt first, const BuildPointsIt last, const size_t pos);
		void dump(const Vector minValues, const Vector maxValues, const size_t pos) const;
		
	public:
		KDTree(const Matrix& cloud);
		virtual Indexes knn(const Vector& query, const Index k, const bool allowSelfMatch);
	};
}

#endif // __NABO_H
