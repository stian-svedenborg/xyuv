/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Stian Valentin Svedenborg
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once
#include <cstdint>
#include <vector>
#include <algorithm>

namespace xyuv {

/** A surface is a two-dimentional array of elements.*/
template<class T>
class surface {
public:
    //! \brief Default constructor, creates an empty surface.
    surface() : _width(0), _height(0), _data() { };

    //! \brief Constructs an \a x_dim by \a y_dim surface.
    surface(uint32_t x_dim, uint32_t y_dim) : _width(x_dim), _height(y_dim), _data(_width * _height) { }

    //! \brief Returns true if the number of accessible elements in the surface equals 0.
    bool empty() const { return !_width || !_height; }

    //! \brief Remove all pixels from the surface.
    //! \details Remove all pixels from the surface, effectively changing the surface to an empty surface.
    void clear() { resize(0, 0); }

    //! \brief Return the width of the surface.
    //! \warning The width is not guaranteed to be 0 for empty surfaces, use empty() to check if the surface is empty.
    uint32_t width() const { return _width; }

    //! \brief Return the height of the surface.
    //! \warning The height is not guaranteed to be 0 for empty surfaces, use empty() to check if the surface is empty.
    uint32_t height() const { return _height; }

    //! \brief Get the value at (x_coord, y_coord), read-only.
    const T &at(uint32_t x_coord, uint32_t y_coord) const { return *(scanline(y_coord) + x_coord); }

    //! \brief Get the value at (x_coord, y_coord), read-write.
    T &at(uint32_t x_coord, uint32_t y_coord) { return *(scanline(y_coord) + x_coord); }

    //! \brief Get the value at (x_coord, y_coord), read-only.
    //! \details This is an alias of at(uint32_t, uint32_t) const.
    const T &get(uint32_t x_coord, uint32_t y_coord) const { return at(x_coord, y_coord); }

    //! \brief Set the value at (x_coord_y_coord).
    void set(uint32_t x_coord, uint32_t y_coord, const T &val) { at(x_coord, y_coord) = val; }

    //! \brief Set the value at (x_coord_y_coord).
    void set(uint32_t x_coord, uint32_t y_coord, T &&val) { at(x_coord, y_coord) = std::move(val); }

    //! \brief Proxy class enabling access through operator[].
    //! \details This class enables this usage of the surface class:
    //! \code{.cpp}
    //! surface<int> surf(3,3);
    //! surf[0][0] = 5;
    //! \endcode
    class row_proxy {
    public:
        T &operator[](uint32_t y_coord) { return parent->at(x_coord, y_coord); }
        const T &operator[](uint32_t y_coord) const { return parent->at(x_coord, y_coord); }

    private:
        row_proxy(surface *parent, uint32_t x_coord) : parent(parent), x_coord(x_coord) { }
        row_proxy(const surface *parent, uint32_t x_coord) : row_proxy(const_cast<surface *>(parent), x_coord) { }

        surface *parent;
        uint32_t x_coord;
    };

    //! \brief \see row_proxy
    row_proxy operator[](uint32_t x_coord) {
        return row_proxy(this, x_coord);
    }

    //! \brief \see row_proxy
    const row_proxy operator[](uint32_t x_coord) const {
        return row_proxy(this, x_coord);
    }

    //! \brief Get a writable pointer to the underlying storage.
    //! \details The elements are stored in row-major order.
    T *data() { return _data.data(); }

    //! \brief Get a writable pointer to the underlying storage.
    //! \details The elements are stored in row-major order.
    const T *data() const { return _data.data(); }

    //! \brief Sample a value from the surface.
    //! \details This will return the "best" representation of the value at coordinate (x,y) which may be between two
    //! discrete samples in the surface.
    //!
    //! Currently the sample method is simple nearest neighbour, but this may change to bi-linear filtering in the future.
    //! \param [in] x X-coordinate of the sample point, must be in the range [0.0, 1.0].
    T sample(float x, float y) const;

    //! \brief Get a raw writable pointer to the data at scanline \a line.
    T *scanline(uint32_t line) { return data() + line * width(); }

    //! \brief Get a raw read-only pointer to the data at scanline \a line.
    const T *scanline(uint32_t line) const { return data() + line * width(); }

    //! \brief Resize the surface.
    //! \todo The data is mangled when resizing the surface. Change this. (Possibly reusing crop)
    void resize(uint32_t w, uint32_t h) {
        // TODO: Make image consistent.
        _data.resize(w * h);
        _width = w;
        _height = h;
    }

    //! \brief Crop the surface.
    //! \details This will crop lines of samples off the edges of the surface. Effectively resizing the surface.
    //!
    //! If all values are removed from the image, the image becomes empty.
    //! \todo Implement this.
    void crop(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom) { }

    //! \brief Resize the surface.
    //! \details The new surface-values will be determined using the sample(float, float) method.
    void scale(uint32_t w, uint32_t h);

    //! \brief Fill the surface with a single value.
    //! \brief This will set all values in the surface equal to \a val.
    void fill(const T &val);

    // iterator 
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;

    //! \brief Get a row major random access iterator to the values.
    iterator begin() { return _data.begin(); }

    //! \brief Get the end iterator.
    iterator end() { return _data.end(); }

    //! \brief Get a row major random access const_iterator to the values.
    const_iterator begin() const { return _data.begin(); }

    //! \brief Get the end iterator.
    const_iterator end() const { return _data.end(); }

    // copy and move semantics.
    surface &operator=(const surface &rhs) = default;
    surface(const surface &rhs) = default;

    surface &operator=(surface &&rhs) {
        this->_width = rhs._width;
        this->_height = rhs._height;
        this->_data = std::move(rhs._data);
        return *this;
    }

    surface(surface &&rhs) : _width(rhs._width), _height(rhs._height), _data(std::move(rhs._data)) { }

private:
    uint32_t _width, _height;
    std::vector<T> _data;
};


template<typename T>
T surface<T>::sample(float x, float y) const {
    x = std::floor(x * width() + 0.5f);
    y = std::floor(y * height() + 0.5f);
    x = std::max(0.0f, std::min<float>(x, width()));
    y = std::max(0.0f, std::min<float>(y, height()));
    // Nearest neighbour "interpolation". (or rather, lack there of.)
    return get(static_cast<uint32_t>(x), static_cast<uint32_t>(y));

}

template<typename T>
void surface<T>::scale(uint32_t w, uint32_t h) {
    surface<T> result(w, h);
    for (uint32_t x = 0; x < w; x++) {
        for (uint32_t y = 0; y < h; y++) {
            result.set(x, y, this->sample(static_cast<float>(x) / w, static_cast<float>(y) / h));
        }
    }
    *this = std::move(result);
}

template<typename T>
void surface<T>::fill(const T &val) {
    for (auto &v : *this) {
        v = val;
    }
}

} // namespace xyuv