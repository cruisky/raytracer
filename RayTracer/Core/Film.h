#pragma once
#include "fwddecl.h"

#include <memory>
#include "Graphics/Color.h"

namespace TX{
	namespace RayTracer{

		enum class FilterType{
			BoxFilter,
			GaussianFilter
		};

		class Film {
		public:
			Film(FilterType filter_t = FilterType::BoxFilter) : width_(0), height_(0), size_(0){ SetFilter(filter_t); };
			Film(int width, int height, FilterType filter_t) { 
				Resize(width, height); 
				SetFilter(filter_t);
			}
			~Film(){}

			inline int Width() const { return width_; }
			inline int Height() const { return height_; }
			inline int Size() const { return size_; }
			
			void Commit(float x, float y, const Color& color);
			void Resize(int width, int height);
			void SetFilter(FilterType filter_t);
			void Reset();						// resets weight and unscaled pixel buffer
			void ResetAll();
			Color Get(int x, int y) const;
			void ScalePixels();
			const Color *Pixels() const;

		private:
			int width_, height_, size_;
			std::unique_ptr<Color[]> pixels_;	// or vector<Color> if Resize() is constantly called
			std::unique_ptr<Color[]> unscaled_pixels_;	// or vector<Color> if Resize() is constantly called
			std::shared_ptr<Filter> filter_;
			std::unique_ptr<float[]> weights_;
		};
	}
}