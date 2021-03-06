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

#ifndef __STATIMO_CORE_MODEL_BUILDER_H_
#define __STATIMO_CORE_MODEL_BUILDER_H_

#include "statismo/core/CommonTypes.h"
#include "statismo/core/DataManager.h"
#include "statismo/core/StatisticalModel.h"
#include "statismo/core/GenericFactory.h"
#include "statismo/core/NonCopyable.h"
#include "statismo/core/Logger.h"

#include <vector>
#include <memory>

/**
 * \defgroup ModelBuilders Statistical model building classes and routines
 */

namespace statismo
{

/**
 * \brief Base abstract class for model builders
 * \ingroup ModelBuilders
 * \ingroup Core
 */
template <typename T>
class ModelBuilder : public NonCopyable
{

public:
  using RepresenterType = Representer<T>;
  using StatisticalModelType = StatisticalModel<T>;
  using DataManagerType = BasicDataManager<T>;
  using DataItemListType = typename DataManagerType::DataItemListType;

  // Values below this tolerance are treated as 0.
  static constexpr double sk_tolerance = 1e-5;

protected:
  MatrixType
  ComputeScores(const MatrixType & matX, const StatisticalModelType * model) const
  {
    MatrixType scores(model->GetNumberOfPrincipalComponents(), matX.rows());
    for (unsigned i = 0; i < scores.cols(); i++)
    {
      scores.col(i) = model->ComputeCoefficientsForSampleVector(matX.row(i));
    }
    return scores;
  }

  MatrixType
  ComputeScores(const DataItemListType & sampleDataList, const StatisticalModelType * model) const
  {
    auto       n = sampleDataList.size();
    MatrixType scores(model->GetNumberOfPrincipalComponents(), n);

    unsigned i{ 0 };
    for (const auto & item : sampleDataList)
    {
      scores.col(i++) = model->ComputeCoefficientsForSampleVector(item->GetSampleVector());
    }

    return scores;
  }

  ModelBuilder() = default;

  ModelInfo
  CollectModelInfo() const;
};

/**
 * \brief Base implementation for model builders
 *
 * \ingroup ModelBuilders
 * \ingroup Core
 */
template <typename T, typename Derived>
class ModelBuilderBase
  : public ModelBuilder<T>
  , public GenericFactory<Derived>
{
public:
  using ObjectFactoryType = GenericFactory<Derived>;

  virtual void
  SetLogger(Logger * logger)
  {
    m_logger = logger;
  }

  virtual void
  Delete()
  {
    delete this;
  }

protected:
  Logger *
  GetLogger() const
  {
    return m_logger;
  }

private:
  Logger * m_logger{ nullptr };
};

} // namespace statismo

#endif
