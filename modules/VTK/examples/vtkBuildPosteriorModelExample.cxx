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

#include "statismo/core/DataManager.h"
#include "statismo/core/PosteriorModelBuilder.h"
#include "statismo/core/StatisticalModel.h"
#include "statismo/core/IO.h"
#include "statismo/core/LoggerMultiHandlersThreaded.h"
#include "statismo/VTK/vtkStandardMeshRepresenter.h"
#include "statismo/VTK/vtkStatismoOutputWindow.h"

#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkNew.h>

#include <iostream>
#include <memory>

//
// This example shows how a partial shape can be reconstructed using a statistical shape model.
//
// \warning This example assumes that the closest point of the partial shape to the model mean is also its
// corresponding point. This is only true if the partial shape is close to the model mean. If this is not the
// case, a more sophisticated method for establishing correspondence needs to be used.
//

using namespace statismo;

namespace
{

using VectorType = statismo::VectorType;
using MatrixType = statismo::MatrixType;
using RepresenterType = statismo::vtkStandardMeshRepresenter;
using StatisticalModelType = statismo::StatisticalModel<vtkPolyData>;
using PosteriorModelBuilderType = statismo::PosteriorModelBuilder<vtkPolyData>;
using DomainType = StatisticalModelType::DomainType;
using DomainPointsConstIterator = DomainType::DomainPointsListType::const_iterator;

vtkSmartPointer<vtkPolyData>
LoadVTKPolyData(const std::string & filename)
{
  vtkNew<vtkPolyDataReader> reader;
  reader->SetFileName(filename.c_str());
  reader->Update();
  return reader->GetOutput();
}

/**
 * Computes the mahalanobis distance of the targetPt, to the model point with the given pointId.
 */
double
MahalanobisDistance(const StatisticalModelType * model, unsigned ptId, const statismo::vtkPoint & targetPt)
{
  statismo::MatrixType cov = model->GetCovarianceAtPoint(ptId, ptId);
  statismo::vtkPoint   meanPt = model->DrawMeanAtPoint(ptId);
  unsigned             pointDim = model->GetRepresenter()->GetDimensions();
  assert(pointDim <= 3);

  VectorType x = VectorType::Zero(pointDim);
  for (unsigned d = 0; d < pointDim; d++)
  {
    x(d) = targetPt[d] - meanPt[d];
  }
  return x.transpose() * cov.inverse() * x;
}

} // namespace


/**
 * Given the inputModel and a partialMesh, the program outputs the posterior model (a statistical model) and
 * its mean, which is the best reconstruction of the partial shape given the model.
 */
int
main(int argc, char ** argv)
{
  if (argc < 5)
  {
    std::cerr << "Usage " << argv[0] << " inputModel  partialShapeMesh posteriorModel reconstructedShape" << std::endl;
    return 1;
  }

  std::string inputModelName(argv[1]);
  std::string partialShapeMeshName(argv[2]);
  std::string posteriorModelName(argv[3]);
  std::string reconstructedShapeName(argv[4]);

  // Background logger
  LoggerMultiHandlersThreaded logger{ std::make_unique<BasicLogHandler>(StdOutLogWriter(), DefaultFormatter()),
                                      LogLevel::LOG_DEBUG,
                                      true };
  logger.Start();
  // Redirect VTK log to Statismo logger
  vtkNew<vtkStatismoOutputWindow> vtkToStatismoLogger;
  vtkToStatismoLogger->SetLogger(&logger);

  vtkOutputWindow::SetInstance(vtkToStatismoLogger);

  try
  {
    auto partialShape = LoadVTKPolyData(partialShapeMeshName);

    auto representer = RepresenterType::SafeCreate();
    representer->SetLogger(&logger);
    auto inputModel = statismo::IO<vtkPolyData>::LoadStatisticalModel(representer.get(), inputModelName);

    StatisticalModelType::PointValueListType constraints;

    // for each point we get the closest point and check how far it is away (in mahalanobis distance).
    // If it is close, we add it as a constraint, otherwise we ignore the remaining points.
    DomainType::DomainPointsListType domainPoints = inputModel->GetDomain().GetDomainPoints();
    for (unsigned ptId = 0; ptId < domainPoints.size(); ptId++)
    {
      statismo::vtkPoint domainPoint = domainPoints[ptId];

      unsigned           closestPointId = ptId;
      statismo::vtkPoint closestPointOnPartialShape = partialShape->GetPoint(closestPointId);
      double             mhdist = MahalanobisDistance(inputModel.get(), ptId, closestPointOnPartialShape);
      if (mhdist < 5)
      {
        constraints.emplace_back(domainPoint, closestPointOnPartialShape);
      }
    }

    // build the new model. In addition to the input model and the constraints, we also specify
    // the inaccuracy of our value (variance of the error).

    auto posteriorModelBuilder = PosteriorModelBuilderType::SafeCreate();
    posteriorModelBuilder->SetLogger(&logger);
    auto constraintModel = posteriorModelBuilder->BuildNewModelFromModel(inputModel.get(), constraints, 0.5);


    // The resulting model is a normal statistical model, from which we could for example sample examples.
    // Here we simply  save it to disk for later use.

    statismo::IO<vtkPolyData>::SaveStatisticalModel(constraintModel.get(), posteriorModelName);
    std::cout << "successfully saved the model to " << posteriorModelName << std::endl;

    // The mean of the constraint model is the optimal reconstruction
    auto pmean = constraintModel->DrawMean();

    vtkNew<vtkPolyDataWriter> writer;
    writer->SetInputData(pmean);
    writer->SetFileName(reconstructedShapeName.c_str());
    writer->Update();
  }
  catch (const statismo::StatisticalModelException & e)
  {
    std::cerr << "Exception occured while building the intenisity model" << std::endl;
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
