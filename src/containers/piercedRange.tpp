/*---------------------------------------------------------------------------*\
 *
 *  bitpit
 *
 *  Copyright (C) 2015-2017 OPTIMAD engineering Srl
 *
 *  -------------------------------------------------------------------------
 *  License
 *  This file is part of bitpit.
 *
 *  bitpit is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License v3 (LGPL)
 *  as published by the Free Software Foundation.
 *
 *  bitpit is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 *  License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with bitpit. If not, see <http://www.gnu.org/licenses/>.
 *
\*---------------------------------------------------------------------------*/

#ifndef __BITPIT_PIERCED_RANGE_TPP__
#define __BITPIT_PIERCED_RANGE_TPP__

namespace bitpit {

/*!
    Constructor.
*/
template<typename value_t, typename id_t, typename value_no_cv_t>
PiercedRange<value_t, id_t, value_no_cv_t>::PiercedRange()
    : m_container(nullptr),
      m_begin_pos(-1),
      m_end_pos(-1)
{
}

/*!
    Constructor.
*/
template<typename value_t, typename id_t, typename value_no_cv_t>
PiercedRange<value_t, id_t, value_no_cv_t>::PiercedRange(container_t *container)
    : m_container(container),
      m_begin_pos(container->cbegin().getRawIndex()),
      m_end_pos(container->cend().getRawIndex())
{
}

/*!
    Constructor.

    \param first is the id of the first element in the range
    \param last is the id of the last element in the range
*/
template<typename value_t, typename id_t, typename value_no_cv_t>
PiercedRange<value_t, id_t, value_no_cv_t>::PiercedRange(container_t *container, id_t first, id_t last)
    : m_container(container),
      m_begin_pos(container->getRawIndex(first)),
      m_end_pos(container->getRawIndex(last) + 1)
{
}

/*!
    Constructor.

    \param begin is the begin of the range
    \param end is the end of the range
*/
template<typename value_t, typename id_t, typename value_no_cv_t>
PiercedRange<value_t, id_t, value_no_cv_t>::PiercedRange(iterator begin, iterator end)
    : m_container(&(begin.getContainer())),
      m_begin_pos(begin.getRawIndex()),
      m_end_pos(end.getRawIndex())
{
    if (&(begin.getContainer()) != &(end.getContainer())) {
        throw std::runtime_error("The two iterators belong to different containers");
    }
}

/*!
    Exchanges the values of the current iterator and
    the iterator recevied as argument.

    \param other the iterator to exchange values with
*/
template<typename value_t, typename id_t, typename value_no_cv_t>
void PiercedRange<value_t, id_t, value_no_cv_t>::swap(PiercedRange &other) noexcept
{
    std::swap(m_container, other.m_container);

    std::swap(m_begin_pos, other.m_begin_pos);
    std::swap(m_end_pos, other.m_end_pos);
}


/*!
    Returns an iterator pointing to the first element in the range.

    \result An iterator pointing to the first element in the range.
*/
template<typename value_t, typename id_t, typename value_no_cv_t>
template<typename U, typename U_no_cv,
         typename std::enable_if<std::is_same<U, U_no_cv>::value, int>::type>
typename PiercedRange<value_t, id_t, value_no_cv_t>::iterator PiercedRange<value_t, id_t, value_no_cv_t>::begin() noexcept
{
    return m_container->getIteratorFromRawIndex(m_begin_pos);
}

/*!
    Returns a constant iterator pointing to the past-the-end element in the
    range.

    \result A constant iterator pointing to the past-the-end element in the
    range.
*/
template<typename value_t, typename id_t, typename value_no_cv_t>
template<typename U, typename U_no_cv,
         typename std::enable_if<std::is_same<U, U_no_cv>::value, int>::type>
typename PiercedRange<value_t, id_t, value_no_cv_t>::iterator PiercedRange<value_t, id_t, value_no_cv_t>::end() noexcept
{
    return m_container->getIteratorFromRawIndex(m_end_pos);
}

/*!
    Returns a constant iterator pointing to the first element in the range.

    \result A constant iterator pointing to the first element in the range.
*/
template<typename value_t, typename id_t, typename value_no_cv_t>
typename PiercedRange<value_t, id_t, value_no_cv_t>::const_iterator PiercedRange<value_t, id_t, value_no_cv_t>::begin() const noexcept
{
    return m_container->getConstIteratorFromRawIndex(m_begin_pos);
}

/*!
    Returns a constant iterator pointing to the past-the-end element in the
    range.

    \result A constant iterator pointing to the past-the-end element in the
    range.
*/
template<typename value_t, typename id_t, typename value_no_cv_t>
typename PiercedRange<value_t, id_t, value_no_cv_t>::const_iterator PiercedRange<value_t, id_t, value_no_cv_t>::end() const noexcept
{
    return m_container->getConstIteratorFromRawIndex(m_end_pos);
}

/*!
    Returns a constant iterator pointing to the first element in the range.

    \result A constant iterator pointing to the first element in the range.
*/
template<typename value_t, typename id_t, typename value_no_cv_t>
typename PiercedRange<value_t, id_t, value_no_cv_t>::const_iterator PiercedRange<value_t, id_t, value_no_cv_t>::cbegin() const noexcept
{
    return m_container->getConstIteratorFromRawIndex(m_begin_pos);
}

/*!
    Returns a constant iterator pointing to the past-the-end element in the
    range.

    \result A constant iterator pointing to the past-the-end element in the
    range.
*/
template<typename value_t, typename id_t, typename value_no_cv_t>
typename PiercedRange<value_t, id_t, value_no_cv_t>::const_iterator PiercedRange<value_t, id_t, value_no_cv_t>::cend() const noexcept
{
    return m_container->getConstIteratorFromRawIndex(m_end_pos);
}

}

#endif
