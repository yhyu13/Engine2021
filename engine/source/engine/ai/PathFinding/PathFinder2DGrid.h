#pragma once

#include <queue>
#include <functional>
#include <vector>
#include <iterator>
#include <algorithm>
#include "PathFindingDefs.h"

namespace longmarch
{
	namespace pathfinding
	{
#define _FLT_LARGE 1e10
#define OPEN_LIST_TYPE 1
#define PREFER_HIGHER_G 1
		template<typename _int = int16_t, typename _ft = float>
		class PathFinder2DGrid
		{
		public:
			using pair = std::pair<_int, _int>;

		private:
			struct pPair
			{
				pPair() = default;
				explicit pPair(_ft _f, _ft _g, _int _i, _int _j)
					:
					f(_f),
					g(_g),
					i(_i),
					j(_j)
				{}
				_ft f, g;
				_int i, j;
			};
			struct pPairComparatorLesser //Unsorted array, min f or higher g first
			{
				bool operator()(const pPair& lhs, const pPair& rhs) noexcept
				{
#if PREFER_HIGHER_G == 1
					if (lhs.f < rhs.f) [[likely]]
					{
						return true;
					}
					else if (lhs.f == rhs.f) [[unlikely]]// Tie breakding, prefer higher g
					{
						return lhs.g > rhs.g;
					}
					else [[likely]]
					{
						return false;
					}
#else
					return lhs.f < rhs.f;
#endif
				}
			};
			struct pPairComparatorGreater //Priority queue, min f or higher g first
			{
				bool operator()(const pPair& lhs, const pPair& rhs) noexcept
				{
#if PREFER_HIGHER_G == 1
					if (lhs.f > rhs.f) [[likely]]
					{
						return true;
					}
					else if (lhs.f == rhs.f) [[unlikely]]// Tie breakding, prefer higher g
					{
						return lhs.g < rhs.g;
					}
					else [[likely]]
					{
						return false;
					}
#else
					return lhs.f > rhs.f;
#endif
				}
			};
			struct pPairComparatorEqual
			{
				bool operator()(const pPair& lhs, const pPair& rhs) noexcept
				{
					return lhs.f == rhs.f;
				}
			};

			struct pPairHash
			{
				std::size_t operator()(const pPair& s) const noexcept
				{
					std::size_t res{ 0 };
					LongMarch_HashCombine(res, s.f);
					LongMarch_HashCombine(res, s.g);
					LongMarch_HashCombine(res, s.i);
					LongMarch_HashCombine(res, s.j);
					return res;
				}
			};

			enum class cell_status_t : uint8_t
			{
				UNSET,
				OK,
				BLOCK,
			};

			struct cell_pair_t
			{
				_int parent_i, parent_j;
			};
			struct cell_f_t
			{
				_ft f;
			};
			struct cell_g_t
			{
				_ft g;
			};
			struct cell_RFW_t
			{
				_ft rfw;
			};

			bool** cell_close{ nullptr };
			cell_status_t** cell_status{ nullptr };
			cell_pair_t** cell_pair{ nullptr };
			cell_f_t** cell_f{ nullptr };
			cell_g_t** cell_g{ nullptr };
			_ft** RFW_dist{ nullptr };
			_int** RFW_next{ nullptr };

			// TODO : consider using a unsorted vector as a priority queue
			using qContainer = std::vector<pPair>;
			using qQueue = std::priority_queue<pPair, std::vector<pPair>, pPairComparatorGreater>;
#if OPEN_LIST_TYPE == 2
			GenericHeap<pPair, pPairHash, pPairComparatorEqual, pPairComparatorGreater> openList; // Generic heap
#elif OPEN_LIST_TYPE == 1
			pPairComparatorGreater comp;
			qQueue openList; // priority queue
#else
			pPairComparatorLesser comp;
			qContainer openList; // unsorted array
#endif
			pair START;
			pair TARGET;

			_int ROW{ -1 };
			_int COL{ -1 };

			bool foundTarget{ false };

		private:
			inline void Allcoate() noexcept
			{
				switch (Method)
				{
				case Method::ASTAR:
				{
					cell_close = (bool**)std::calloc(1, ROW * sizeof(bool*));
					for (int i = 0; i < ROW; ++i)
					{
						cell_close[i] = (bool*)std::calloc(1, COL * sizeof(bool));
					}
					cell_status = (cell_status_t**)std::calloc(1, ROW * sizeof(cell_status_t*));
					for (int i = 0; i < ROW; ++i)
					{
						cell_status[i] = (cell_status_t*)std::calloc(1, COL * sizeof(cell_status_t));
					}
					cell_pair = (cell_pair_t**)std::calloc(1, ROW * sizeof(cell_pair_t*));
					for (int i = 0; i < ROW; ++i)
					{
						cell_pair[i] = (cell_pair_t*)std::calloc(1, COL * sizeof(cell_pair_t));
					}
					cell_f = (cell_f_t**)std::calloc(1, ROW * sizeof(cell_f_t*));
					for (int i = 0; i < ROW; ++i)
					{
						cell_f[i] = (cell_f_t*)std::calloc(1, COL * sizeof(cell_f_t));
					}
					cell_g = (cell_g_t**)std::calloc(1, ROW * sizeof(cell_g_t*));
					for (int i = 0; i < ROW; ++i)
					{
						cell_g[i] = (cell_g_t*)std::calloc(1, COL * sizeof(cell_g_t));
					}
				}
				break;
				case Method::ROY_FLOYD_WARSHALL:
				{
					RFW_dist = (_ft**)std::calloc(1, ROW * COL * sizeof(_ft*));
					for (int i = 0; i < ROW * COL; ++i)
					{
						RFW_dist[i] = (_ft*)std::calloc(1, ROW * COL * sizeof(_ft));
					}
					RFW_next = (_int**)std::calloc(1, ROW * COL * sizeof(_int*));
					for (int i = 0; i < ROW * COL; ++i)
					{
						RFW_next[i] = (_int*)std::calloc(1, ROW * COL * sizeof(_int));
					}
				}
				break;
				default:
					break;
				}
			}
			//! Utility
			inline bool IsValid(_int row, _int col) noexcept
			{
				return (unsigned)row < ROW && (unsigned)col < COL;
			}
			//! Utility
			inline _ft IsTarget(_int row, _int col) noexcept
			{
				return row == TARGET.first && col == TARGET.second;
			}
			//! H value
			inline _ft CauculateHeuristicValue(_int row, _int col) noexcept
			{
				_ft dx = std::abs(row - TARGET.first);
				_ft dy = std::abs(col - TARGET.second);
				switch (HeuristicOption)
				{
				[[likely]] case Heuristic::OCTILE:
					// Octile
					return D * (dx + dy) + (D2 - 2.0 * D) * std::min(dx, dy);
				case  Heuristic::CHEBYSHEV:
					// Chebyshev
					return D * std::max(dx, dy);
					//return D * (dx + dy) + (D - 2.0 * D) * std::min(dx, dy); // (equivalent)
				case  Heuristic::MANHATTAN:
					// L1
					return D * (dx + dy);
				case  Heuristic::EUCLIDEAN:
					// L2
					return D * std::sqrt(dx * dx + dy * dy);
				}
			}
			//! Core function of the A star algorithm
			template<bool isDiag>
			inline bool UpdateCell(_int i, _int j, _int parent_i, _int parent_j) noexcept
			{
				if (IsValid(i, j))
				{
					// Avoid invoking potentially expensive IsBlocked function by introducing cell status buffer
					// Set cell status
					auto& status = cell_status[i][j];
					if (status == cell_status_t::UNSET)
					{
						if (IsBlocked(i, j))
						{
							status = cell_status_t::BLOCK;
						}
						else
						{
							status = cell_status_t::OK;
						}
					}
					// Do more block checking for diagonal cells
					if constexpr (isDiag)
					{
						{
							auto& status = cell_status[i][parent_j];
							if (status == cell_status_t::UNSET)
							{
								if (IsBlocked(i, parent_j))
								{
									status = cell_status_t::BLOCK;
								}
								else
								{
									status = cell_status_t::OK;
								}
							}
							if (status == cell_status_t::BLOCK)
							{
								return false;
							}
						}
						{
							auto& status = cell_status[parent_i][j];
							if (status == cell_status_t::UNSET)
							{
								if (IsBlocked(parent_i, j))
								{
									status = cell_status_t::BLOCK;
								}
								else
								{
									status = cell_status_t::OK;
								}
							}
							if (status == cell_status_t::BLOCK)
							{
								return false;
							}
						}
					}
					if (status == cell_status_t::BLOCK)
					{
						return false;
					}

					// return success on finding the target
					if (auto& ij = cell_pair[i][j]; IsTarget(i, j)) [[unlikely]]
					{
						// Set the Parent of the destination cell
						ij.parent_i = parent_i;
						ij.parent_j = parent_j;
						foundTarget = true;
						return true;
					}
						// Ignore closed list
					else if (!cell_close[i][j]) [[likely]]
					{
						_ft d;
						if constexpr (isDiag)
						{
							d = D2;
						}
						else
						{
							d = D;
						}
						auto g_ = cell_g[parent_i][parent_j].g + d;
						auto f_ = (g_ + CauculateHeuristicValue(i, j) * HMultiplier);
						if (auto& f = cell_f[i][j]; f.f > f_)
						{
							f.f = f_;
							cell_g[i][j].g = g_;
							ij.parent_i = parent_i;
							ij.parent_j = parent_j;
	#if OPEN_LIST_TYPE == 2
							openList.emplace(f_, g_, i, j);
	#elif OPEN_LIST_TYPE == 1
							openList.emplace(f_, g_, i, j);
	#else
							openList.emplace_back(f_, g_, i, j);
	#endif
							SetOpenListColor(i, j);
						}
					}
				}
				return false;
			}

		public:
			std::function<void(_int, _int)> SetOpenListColor{};
			std::function<void(_int, _int)> SetClosedListColor{};
			std::function<bool(_int, _int)> IsBlocked{};
			std::function<void()> TracePath{};
			Heuristic HeuristicOption{ Heuristic::OCTILE };
			PathResult Status{ PathResult::IDLE };
			Method Method{ Method::ASTAR };
			std::vector<pair> Result; //!< The bottom (first element) of the stack is target and the top (last element) of the stack is the start
			_ft HMultiplier{ 1.0 };
			const _ft D{ 1.0 };
			const _ft D2{ 1.414213562373095 };

		public:
			PathFinder2DGrid(const PathFinder2DGrid&) = delete;
			PathFinder2DGrid(PathFinder2DGrid&&) = delete;
			PathFinder2DGrid& operator=(const PathFinder2DGrid&) = delete;
			PathFinder2DGrid& operator=(PathFinder2DGrid&&) = delete;

			PathFinder2DGrid() noexcept
			{
#if OPEN_LIST_TYPE == 2
				openList.reserve(128 * 128);
#elif OPEN_LIST_TYPE == 1
				qContainer containerVec;
				containerVec.reserve(128 * 128);
				openList = std::move(qQueue(comp, std::move(containerVec)));
#else
				openList.reserve(128 * 128);
#endif
				Result.reserve(128 * 128);
			}
			~PathFinder2DGrid() noexcept
			{
				Release();
			}

			inline void Release() noexcept
			{
				switch (Method)
				{
				case Method::ASTAR:
				{
					for (int i = 0; i < ROW; ++i)
					{
						std::free(cell_close[i]);
						std::free(cell_status[i]);
						std::free(cell_pair[i]);
						std::free(cell_f[i]);
						std::free(cell_g[i]);
					}
					std::free(cell_close);
					std::free(cell_status);
					std::free(cell_pair);
					std::free(cell_f);
					std::free(cell_g);
				}
				break;
				case Method::ROY_FLOYD_WARSHALL:
				{
					for (int i = 0; i < ROW * COL; ++i)
					{
						std::free(RFW_dist[i]);
						std::free(RFW_next[i]);
					}
					std::free(RFW_dist);
					std::free(RFW_next);
				}
				break;
				default:
					break;
				}
			}

			inline void Init(int row, int col) noexcept
			{
				switch (Method)
				{
				case Method::ASTAR:
				{
					InitAStar(row, col);
				}
				break;
				case Method::ROY_FLOYD_WARSHALL:
				{
					InitRFW(row, col);
				}
				break;
				default:
					break;
				}
			}

			inline void InitAStar(int row, int col) noexcept
			{
				ROW = row;
				COL = col;
				Allcoate();
			}

			inline void InitRFW(int row, int col) noexcept
			{
				ROW = row;
				COL = col;
				Allcoate();
				{
					// Assume bidirectional and no negative cost so that we can skip the diagonal negative value check which determines the existence of a negative cycle
					// Initialize pairs
					for (_int i = 0; i < ROW * COL; ++i)
					{
						for (_int j = 0; j < ROW * COL; ++j)
						{
							RFW_dist[i][j] = _FLT_LARGE;
							RFW_next[i][j] = -1;
						}
					}
					for (_int i = 0; i < ROW * COL; ++i)
					{
						auto start = pair{ i / COL, i % COL };
						if (!IsBlocked(start.first, start.second))
						{
							for (_int j = 0; j < ROW * COL; ++j)
							{
								auto target = pair{ j / COL, j % COL };
								// Blocked
								if (IsBlocked(target.first, target.second))
								{
									continue;
								}
								else if (i == j)
								{
									RFW_dist[i][i] = 0;
									RFW_next[i][i] = i;
								}
								// Same row
								else if (start.first == target.first && abs(start.second - target.second) == 1)
								{
									RFW_dist[i][j] = D;
									RFW_next[i][j] = j;
								}
								// Same col
								else if (start.second == target.second && abs(start.first - target.first) == 1)
								{
									RFW_dist[i][j] = D;
									RFW_next[i][j] = j;
								}
								else
								{   // Diagonal
									auto diag_row = (target.first - start.first);
									auto diag_col = (target.second - start.second);
									if (abs(diag_row) == 1 && abs(diag_col) == 1 && (!IsBlocked(start.first, target.second) || !IsBlocked(target.first, start.second)))
									{
										RFW_dist[i][j] = D2;
										RFW_next[i][j] = j;
									}
								}
							}
						}
					}
					// Dynamic programming the solution of all-pairs shortest path table
					for (_int k = 0; k < ROW * COL; ++k)
					{
						for (_int i = 0; i < ROW * COL; ++i)
						{
							if (k == i)
							{
								continue;
							}
							for (_int j = 0; j < ROW * COL; ++j)
							{
								auto old_ = RFW_dist[i][j];
								auto new_ = RFW_dist[i][k] + RFW_dist[k][j];
								if (old_ > new_)
								{
									RFW_dist[i][j] = new_;
									auto next = RFW_next[i][k];
									RFW_next[i][j] = next;
								}
							}
						}
					}
				}
			}

			inline bool PrepareForSeacrh(pair start, pair target) noexcept
			{
				START = start;
				TARGET = target;
				if (!IsValid(start.first, start.second) || !IsValid(target.first, target.second))
				{
					return false;
				}
				if (IsBlocked(start.first, start.second) || IsBlocked(target.first, target.second))
				{
					return false;
				}
				if (IsTarget(start.first, start.second))
				{
					return false;
				}
				switch (Method)
				{
				case Method::ASTAR:
				{
					// Reset cells
					_int i, j;
					for (i = 0; i < ROW; ++i)
					{
						auto& cell_close_ = cell_close[i];
						auto& cell_status_ = cell_status[i];
						auto& cell_pair_ = cell_pair[i];
						auto& cell_f_ = cell_f[i];
						auto& cell_g_ = cell_g[i];
						for (j = 0; j < COL; ++j)
						{
							cell_close_[j] = false;
							cell_status_[j] = cell_status_t::UNSET;
							cell_pair_[j].parent_i = cell_pair_[j].parent_j = -1;
							cell_f_[j].f = cell_g_[j].g = _FLT_LARGE;
						}
					}
					// Initialize to the starting point
					i = start.first;
					j = start.second;
					cell_pair[i][j].parent_i = i;
					cell_pair[i][j].parent_j = j;
					cell_f[i][j].f = cell_g[i][j].g = 0.0;

					// Put starting point on the open list
#if OPEN_LIST_TYPE == 2
					openList.reset();
					openList.emplace(0.0, 0.0, i, j);
#elif OPEN_LIST_TYPE == 1
					while (!openList.empty()) { openList.pop(); }
					openList.emplace(0.0, 0.0, i, j);
#else
					openList.clear();
					openList.emplace_back(0.0, 0.0, i, j);
#endif
					Result.clear();
					foundTarget = false;
					return true;
				}
				break;
				case Method::ROY_FLOYD_WARSHALL:
				{
					Result.clear();
					foundTarget = false;
					return true;
				}
				break;
				default:
					break;
				}
			}

			//! Only process one step of the A star search (great for debugging)
			inline void OneStep() noexcept
			{
				switch (Method)
				{
				case Method::ASTAR:
				{
					_int i, j;
					if (openList.empty())
					{
						Status = PathResult::IMPOSSIBLE;
						return;
					}
					else
					{
						// pop from open list and assign to closed list
#if OPEN_LIST_TYPE == 2
						const auto& p = openList.remove();
						i = p.i;
						j = p.j;
#elif OPEN_LIST_TYPE == 1
						const auto& p = openList.top();
						i = p.i;
						j = p.j;
						openList.pop();
#else
						auto p_iter = std::min_element(openList.begin(), openList.end(), comp);
						i = p_iter->i;
						j = p_iter->j;
						std::iter_swap(p_iter, openList.end() - 1);
						openList.pop_back();
#endif
						cell_close[i][j] = true;
						SetClosedListColor(i, j);

						// Execute the same row all-together for better cache coherrence
						// Top rows
						if (UpdateCell<true>(i - 1, j - 1, i, j)) goto OneStep_quit;
						if (UpdateCell<false>(i - 1, j, i, j)) goto OneStep_quit;
						if (UpdateCell<true>(i - 1, j + 1, i, j)) goto OneStep_quit;
						// Middle rows
						if (UpdateCell<false>(i, j - 1, i, j)) goto OneStep_quit;
						if (UpdateCell<false>(i, j + 1, i, j)) goto OneStep_quit;
						// Bottom rows
						if (UpdateCell<true>(i + 1, j - 1, i, j)) goto OneStep_quit;
						if (UpdateCell<false>(i + 1, j, i, j)) goto OneStep_quit;
						if (UpdateCell<true>(i + 1, j + 1, i, j)) goto OneStep_quit;

						Status = PathResult::PROCESSING;
						return;
				}
				OneStep_quit:
					if (foundTarget)
					{
						Status = PathResult::COMPLETE;
					}
					else
					{
						Status = PathResult::IMPOSSIBLE;
					}
				}
				break;
				case Method::ROY_FLOYD_WARSHALL:
				{
					auto v = START.first * COL + START.second;
					auto u = TARGET.first * COL + TARGET.second;
					auto next = RFW_next[v][u];
					foundTarget = (next != -1);
					if (foundTarget)
					{
						auto dist = RFW_dist[v][u];
						if (dist == _FLT_LARGE)
						{
							throw std::runtime_error("Sanity check failed!");
						}
						Status = PathResult::COMPLETE;
					}
					else
					{
						Status = PathResult::IMPOSSIBLE;
					}
				}
				break;
				default:
					break;
			}
		}

			inline void Search() noexcept
			{
				switch (Method)
				{
				case Method::ASTAR:
				{
					_int i, j;
					while (!openList.empty())
					{
						// pop from open list and assign to closed list
#if OPEN_LIST_TYPE == 2
						const auto& p = openList.remove();
						i = p.i;
						j = p.j;
#elif OPEN_LIST_TYPE == 1
						const auto& p = openList.top();
						i = p.i;
						j = p.j;
						openList.pop();
#else
						auto p_iter = std::min_element(openList.begin(), openList.end(), comp);
						i = p_iter->i;
						j = p_iter->j;
						std::iter_swap(p_iter, openList.end() - 1);
						openList.pop_back();
#endif
						cell_close[i][j] = true;
						SetClosedListColor(i, j);

						// Execute the same row all-together for better cache coherrence
						// top rows
						if (UpdateCell<true>(i - 1, j - 1, i, j)) break;
						if (UpdateCell<false>(i - 1, j, i, j)) break;
						if (UpdateCell<true>(i - 1, j + 1, i, j)) break;
						// middle rows
						if (UpdateCell<false>(i, j - 1, i, j)) break;
						if (UpdateCell<false>(i, j + 1, i, j)) break;
						// bottom rows
						if (UpdateCell<true>(i + 1, j - 1, i, j)) break;
						if (UpdateCell<false>(i + 1, j, i, j)) break;
						if (UpdateCell<true>(i + 1, j + 1, i, j)) break;
				}
					if (foundTarget)
					{
						Status = PathResult::COMPLETE;
					}
					else
					{
						Status = PathResult::IMPOSSIBLE;
					}
				}
				break;
				case Method::ROY_FLOYD_WARSHALL:
				{
					auto v = START.first * COL + START.second;
					auto u = TARGET.first * COL + TARGET.second;
					auto next = RFW_next[v][u];
					foundTarget = (next != -1);
					if (foundTarget)
					{
						auto dist = RFW_dist[v][u];
						if (dist == _FLT_LARGE)
						{
							throw std::runtime_error("Sanity check failed!");
						}
						Status = PathResult::COMPLETE;
					}
					else
					{
						Status = PathResult::IMPOSSIBLE;
					}
				}
				break;
				default:
					break;
			}
	}

			inline void CollectRawPathFindingReuslt() noexcept
			{
				switch (Method)
				{
				case Method::ASTAR:
				{
					auto [row, col] = TARGET;
					// The bottom of the stack is target and the top of the stack is the start
					Result.clear();
					while (!(cell_pair[row][col].parent_i == row && cell_pair[row][col].parent_j == col))
					{
						Result.emplace_back(row, col);
						auto temp_row = cell_pair[row][col].parent_i;
						auto temp_col = cell_pair[row][col].parent_j;
						row = temp_row;
						col = temp_col;
					}
					Result.emplace_back(row, col);
				}
				break;
				case Method::ROY_FLOYD_WARSHALL:
				{
					auto v = START.first * COL + START.second;
					auto u = TARGET.first * COL + TARGET.second;
					auto [row, col] = START;
					Result.clear();
					while (v != u)
					{
						Result.emplace_back(row, col);
						v = RFW_next[v][u];
						row = v / COL;
						col = v % COL;
					}
					Result.emplace_back(row, col);
					std::reverse(std::begin(Result), std::end(Result));
				}
				break;
				default:
					break;
				}
			}

			inline void RubberBandingRawResult() noexcept
			{
				// from target to start, we try to short cut the reuslt path
				auto head_it = Result.begin();
				// rubber banding stops if we have fewer than 3 points
				while (std::distance(head_it, Result.end()) >= 3)
				{
					auto mid_it = head_it + 1;
					auto end_it = head_it + 2;

					auto head_pt = *head_it;
					auto mid_pt = *mid_it;
					auto end_pt = *end_it;

					// A simple and stupid algorithm that checks if there is are blocks within the bounding box, if there is none, then we can safely eliminate the midpoint
					bool noBlocked = true;
					auto minmax_i = std::minmax({ head_pt.first ,mid_pt.first ,end_pt.first });
					auto minmax_j = std::minmax({ head_pt.second ,mid_pt.second ,end_pt.second });
					for (_int i = minmax_i.first; i <= minmax_i.second; ++i)
					{
						for (_int j = minmax_j.first; j <= minmax_j.second; ++j)
						{
							if (IsValid(i, j) && IsBlocked(i, j))
							{
								noBlocked = false;
								break;
							}
						}
					}
					if (noBlocked)
					{
						// Result.erase(mid_it) // Should be equivalent of just erase mid point because only iterator equald and after mid pointer are invalidated
						head_it = Result.erase(mid_it) - 1;
					}
					else
					{
						++head_it;
					}
				}
			}
};

#undef _FLT_LARGE
#undef OPEN_LIST_TYPE
#undef PREFER_HIGHER_G
	}
}