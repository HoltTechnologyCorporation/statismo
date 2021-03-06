/*
 * Copyright (c) 2015 University of Basel
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

#include "utils/statismoLoggingUtils.h"
#include "utils/statismoBuildPosteriorModelUtils.h"

#include "statismo/ITK/itkStandardImageRepresenter.h"
#include "statismo/ITK/itkStandardMeshRepresenter.h"
#include "statismo/ITK/itkIO.h"
#include "statismo/ITK/itkStatisticalModel.h"

#include "lpo.h"

#include <itkImage.h>
#include <itkMesh.h>
#include <itkPointsLocator.h>
#include <itkVersorRigid3DTransform.h>

namespace po = lpo;
using namespace std;

namespace
{
constexpr unsigned gk_dimensionality3D = 3;
constexpr unsigned gk_dimensionality2D = 2;

struct ProgramOptions
{
  string      strModelType;
  string      strInputModelFileName;
  string      strOutputModelFileName;
  string      strInputMeshFileName;
  string      strInputFixedLandmarksFileName;
  string      strInputMovingLandmarksFileName;
  double      dVariance{ 0.0 };
  unsigned    uNumberOfDimensions{ 0 };
  bool        bIsQuiet{ false };
  std::string strLogFile{ "" };
};

bool
IsOptionsConflictPresent(ProgramOptions & opt)
{
  statismo::utils::ToLower(opt.strModelType);
  return ((!opt.strInputFixedLandmarksFileName.empty()) ^ (!opt.strInputMovingLandmarksFileName.empty())) ||
         !((!opt.strInputFixedLandmarksFileName.empty()) ^ (!opt.strInputMeshFileName.empty())) ||
         (opt.strModelType != "shape" && opt.strModelType != "deformation") || opt.strInputModelFileName.empty() ||
         opt.strOutputModelFileName.empty() ||
         (opt.strModelType == "deformation" && !opt.strInputMeshFileName.empty()) ||
         (opt.strModelType == "shape" && opt.uNumberOfDimensions != 3);
}

void
BuildPosteriorShapeModel(const ProgramOptions & opt, statismo::Logger * logger)
{
  using DataType = itk::Mesh<float, gk_dimensionality3D>;
  using StatisticalModelType = itk::StatisticalModel<DataType>;
  using RepresenterType = itk::StandardMeshRepresenter<float, gk_dimensionality3D>;

  auto representer = RepresenterType::New();
  representer->SetLogger(logger);
  auto model = itk::StatismoIO<DataType>::LoadStatisticalModel(representer.GetPointer(), opt.strInputModelFileName);

  StatisticalModelType::Pointer constrainedModel;
  if (opt.strInputMeshFileName.empty())
  {
    using PointsLocatorType = itk::PointsLocator<DataType::PointsContainer>;
    constrainedModel = statismo::cli::BuildPosteriorShapeModel<DataType, StatisticalModelType, PointsLocatorType>(
      model, opt.strInputFixedLandmarksFileName, opt.strInputMovingLandmarksFileName, opt.dVariance, logger);
  }
  else
  {
    using MeshReaderType = itk::MeshFileReader<DataType>;
    auto meshReader = MeshReaderType::New();
    meshReader->SetFileName(opt.strInputMeshFileName);
    meshReader->Update();
    DataType::Pointer meshInCorrespondence = meshReader->GetOutput();
    constrainedModel = statismo::cli::BuildPosteriorShapeModel<DataType, StatisticalModelType>(
      model, meshInCorrespondence, opt.dVariance, logger);
  }

  itk::StatismoIO<DataType>::SaveStatisticalModel(constrainedModel, opt.strOutputModelFileName);
}

template <unsigned DIMENSIONALITY>
void
BuildPosteriorDeformationModel(const ProgramOptions & opt, statismo::Logger * logger)
{
  using VectorPixelType = itk::Vector<float, DIMENSIONALITY>;
  using DataType = itk::Image<VectorPixelType, DIMENSIONALITY>;
  using StatisticalModelType = itk::StatisticalModel<DataType>;
  using RepresenterType = itk::StandardImageRepresenter<VectorPixelType, DIMENSIONALITY>;

  auto representer = RepresenterType::New();
  representer->SetLogger(logger);
  auto model = itk::StatismoIO<DataType>::LoadStatisticalModel(representer.GetPointer(), opt.strInputModelFileName);

  itk::StatismoIO<DataType>::SaveStatisticalModel(
    statismo::cli::BuildPosteriorDeformationModel<DataType, StatisticalModelType>(
      model, opt.strInputFixedLandmarksFileName, opt.strInputMovingLandmarksFileName, opt.dVariance, logger),
    opt.strOutputModelFileName);
}
} // namespace

int
main(int argc, char ** argv)
{
  ProgramOptions                                     poParameters;
  po::program_options<std::string, unsigned, double> parser{ argv[0], "Program help:" };

  parser
    .add_opt<std::string>({ "type",
                            "t",
                            "Specifies the type of the model: SHAPE and DEFORMATION are the two available types",
                            &poParameters.strModelType,
                            "SHAPE" },
                          true)
    .add_opt<unsigned>({ "dimensionality",
                         "d",
                         "DIMENSIONALITY of the input image (only available if you're building a deformation model)",
                         &poParameters.uNumberOfDimensions,
                         3,
                         2,
                         3 },
                       true)
    .add_opt<std::string>({ "input-file",
                            "i",
                            "The path to the model file from which the posterior model will be built.",
                            &poParameters.strInputModelFileName,
                            "" },
                          true)
    .add_opt<std::string>({ "landmarks-fixed",
                            "f",
                            "Name of the file where the fixed Landmarks are saved.",
                            &poParameters.strInputFixedLandmarksFileName,
                            "" })
    .add_opt<std::string>({ "landmarks-moving",
                            "m",
                            "Name of the file where the moving Landmarks are saved.",
                            &poParameters.strInputMovingLandmarksFileName,
                            "" })
    .add_opt<std::string>({ "corresponding-mesh",
                            "c",
                            "Path to the Mesh in correspondence. This is only available if the type is SHAPE.",
                            &poParameters.strInputMeshFileName,
                            "" })
    .add_opt<double>({ "landmarks-variance",
                       "v",
                       "The variance that will be used to build the posterior model.",
                       &poParameters.dVariance,
                       1.0f })
    .add_pos_opt<std::string>(
      { "Name of the output file where the posterior model will be written to.", &poParameters.strOutputModelFileName })
    .add_flag({ "quiet", "q", "Quiet mode (no log).", &poParameters.bIsQuiet, false })
    .add_opt<std::string>({ "log-file", "", "Path to the log file.", &poParameters.strLogFile, "" }, false);


  if (!parser.parse(argc, argv))
  {
    return EXIT_FAILURE;
  }

  if (IsOptionsConflictPresent(poParameters))
  {
    cerr << "A conflict in the options exists or insufficient options were set." << endl;
    cout << parser << endl;
    return EXIT_FAILURE;
  }

  try
  {
    std::unique_ptr<statismo::Logger> logger{ nullptr };
    if (!poParameters.bIsQuiet)
    {
      logger = statismo::cli::CreateLogger(poParameters.strLogFile);
    }

    if (poParameters.strModelType == "shape")
    {
      BuildPosteriorShapeModel(poParameters, logger.get());
    }
    else
    {
      if (poParameters.uNumberOfDimensions == gk_dimensionality2D)
      {
        BuildPosteriorDeformationModel<gk_dimensionality2D>(poParameters, logger.get());
      }
      else
      {
        BuildPosteriorDeformationModel<gk_dimensionality3D>(poParameters, logger.get());
      }
    }
  }
  catch (const ifstream::failure & e)
  {
    cerr << "Could not read a file:" << endl;
    cerr << e.what() << endl;
    return EXIT_FAILURE;
  }
  catch (itk::ExceptionObject & e)
  {
    cerr << "Could not build the posterior model:" << endl;
    cerr << e.what() << endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
