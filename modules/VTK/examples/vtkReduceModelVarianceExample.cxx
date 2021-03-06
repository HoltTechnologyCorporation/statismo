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

#include "statismo/core/ReducedVarianceModelBuilder.h"
#include "statismo/core/StatisticalModel.h"
#include "statismo/core/IO.h"
#include "statismo/VTK/vtkStandardMeshRepresenter.h"

#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkNew.h>

#include <iostream>
#include <memory>

using namespace statismo;


// illustrates how to reduce the variance of a model
int
main(int argc, char ** argv)
{
  if (argc < 3)
  {
    std::cerr << "Usage " << argv[0] << " inputmodel outputmodel" << std::endl;
    return 1;
  }
  std::string inputModelName(argv[1]);
  std::string outputModelName(argv[2]);


  // All the statismo classes have to be parameterized with the RepresenterType.
  // For building a shape model with vtk, we use the vtkPolyDataRepresenter.
  using RepresenterType = vtkStandardMeshRepresenter;
  using ReducedVarianceModelBuilderType = ReducedVarianceModelBuilder<vtkPolyData>;

  try
  {

    // To load a model, we call the static Load method, which returns (a pointer to) a
    // new StatisticalModel object
    auto representer = RepresenterType::SafeCreate();
    auto model = statismo::IO<vtkPolyData>::LoadStatisticalModel(representer.get(), inputModelName);
    std::cout << "loaded model with variance of " << model->GetPCAVarianceVector().sum() << std::endl;

    auto reducedVarModelBuilder = ReducedVarianceModelBuilderType::SafeCreate();

    // build a model with only half the variance
    auto reducedModel = reducedVarModelBuilder->BuildNewModelWithVariance(model.get(), 0.5);
    std::cout << "new model has variance of " << reducedModel->GetPCAVarianceVector().sum() << std::endl;

    statismo::IO<vtkPolyData>::SaveStatisticalModel(reducedModel.get(), outputModelName);
  }
  catch (const StatisticalModelException & e)
  {
    std::cerr << "Exception occured while building the shape model" << std::endl;
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
