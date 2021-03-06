/*
 * This file is part of the statismo library.
 *
 * Author: Marcel Luethi (marcel.luethi@unibas.ch)
 *
 * Copyright (c) 2011 University of Basel
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * Neither the name of the project's author nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __STATIMO_CORE_DOMAIN_H_
#define __STATIMO_CORE_DOMAIN_H_

#include <vector>

namespace statismo
{

/**
 * \brief Represents the domain on which a statistical model is defined
 *
 * A domain is simply a list of points.
 *
 * \ingroup Core
 */
template <typename PointType>
class Domain
{
public:
  using DomainPointsListType = std::vector<PointType>;

  /**
   * \brief Create an empty domain
   */
  Domain() = default;

  /**
   * \brief Create a new domain from the given list of points (copy version)
   */
  explicit Domain(DomainPointsListType domainPoints)
    : m_domainPoints(std::move(domainPoints))
  {}

  /**
   * \brief Create a new domain from the given list of points (move version)
   */
  explicit Domain(DomainPointsListType && domainPoints)
    : m_domainPoints(std::move(domainPoints))
  {}

  /**
   * \brief Returns a list of points that define the domain
   */
  DomainPointsListType
  GetDomainPoints() const
  {
    return m_domainPoints;
  }

  /**
   * \brief Returns the number of points of the domain
   */
  std::size_t
  GetNumberOfPoints() const
  {
    return m_domainPoints.size();
  }

private:
  DomainPointsListType m_domainPoints;
};

} // namespace statismo

#endif
