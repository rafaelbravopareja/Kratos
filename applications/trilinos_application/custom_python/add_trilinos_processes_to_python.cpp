#if defined(KRATOS_PYTHON)
// External includes
#include "custom_python/add_trilinos_processes_to_python.h"

//Trilinos includes
#include "mpi.h"
#include "Epetra_MpiComm.h"
#include "Epetra_Comm.h"
#include "Epetra_Map.h"
#include "Epetra_Vector.h"
#include "Epetra_FECrsGraph.h"
#include "Epetra_FECrsMatrix.h"
#include "Epetra_FEVector.h"
#include "Epetra_IntSerialDenseVector.h"
#include "Epetra_SerialDenseMatrix.h"


// Project includes
#include "includes/define.h"
#include "includes/model_part.h"
#include "processes/process.h"
#include "trilinos_application.h"
#include "trilinos_space.h"
#include "spaces/ublas_space.h"


#include "custom_processes/trilinos_spalart_allmaras_turbulence_model.h"
#include "custom_processes/trilinos_stokes_initialization_process.h"
#include "custom_processes/trilinos_variational_distance_calculation_process.h"
#include "../FluidDynamicsApplication/custom_processes/spalart_allmaras_turbulence_model.h"
#include "../FluidDynamicsApplication/custom_processes/stokes_initialization_process.h"
#include "linear_solvers/linear_solver.h"

namespace Kratos
{

namespace Python
{

using namespace pybind11;

typedef TrilinosSpace<Epetra_FECrsMatrix, Epetra_FEVector> TrilinosSparseSpaceType;
typedef UblasSpace<double, Matrix, Vector> TrilinosLocalSpaceType;
typedef LinearSolver<TrilinosSparseSpaceType, TrilinosLocalSpaceType > TrilinosLinearSolverType;

void AddProcesses(pybind11::module& m)
{
    typedef SpalartAllmarasTurbulenceModel<TrilinosSparseSpaceType, TrilinosLocalSpaceType, TrilinosLinearSolverType> BaseSpAlModelType;

    class_<BaseSpAlModelType, BaseSpAlModelType::Pointer, Process>(m, "TrilinosBaseSpAlModel" );

    // Turbulence models
    class_< TrilinosSpalartAllmarasTurbulenceModel< TrilinosSparseSpaceType, TrilinosLocalSpaceType, TrilinosLinearSolverType >, BaseSpAlModelType >
    (m,"TrilinosSpalartAllmarasTurbulenceModel")
    .def(init < Epetra_MpiComm&, ModelPart&, TrilinosLinearSolverType::Pointer, unsigned int, double, unsigned int, bool, unsigned int>())
    .def("ActivateDES", &SpalartAllmarasTurbulenceModel< TrilinosSparseSpaceType, TrilinosLocalSpaceType, TrilinosLinearSolverType >::ActivateDES)
    .def("AdaptForFractionalStep", &SpalartAllmarasTurbulenceModel< TrilinosSparseSpaceType, TrilinosLocalSpaceType, TrilinosLinearSolverType >::AdaptForFractionalStep)
    ;

    typedef StokesInitializationProcess<TrilinosSparseSpaceType, TrilinosLocalSpaceType, TrilinosLinearSolverType> BaseStokesInitializationType;

    class_<BaseStokesInitializationType, BaseStokesInitializationType::Pointer, Process>
            (m, "TrilinosBaseStokesInitialization" )
            .def("SetConditions",&BaseStokesInitializationType::SetConditions);

    class_<TrilinosStokesInitializationProcess< TrilinosSparseSpaceType, TrilinosLocalSpaceType, TrilinosLinearSolverType >, BaseStokesInitializationType >
            (m,"TrilinosStokesInitializationProcess")
            .def(init<Epetra_MpiComm&, ModelPart::Pointer,TrilinosLinearSolverType::Pointer, unsigned int, const Kratos::Variable<int>& >())
            ;

    typedef VariationalDistanceCalculationProcess<2,TrilinosSparseSpaceType, TrilinosLocalSpaceType, TrilinosLinearSolverType> BaseDistanceCalculationType2D;
    typedef VariationalDistanceCalculationProcess<3,TrilinosSparseSpaceType, TrilinosLocalSpaceType, TrilinosLinearSolverType> BaseDistanceCalculationType3D;

    class_<BaseDistanceCalculationType2D, BaseDistanceCalculationType2D::Pointer, Process>(m,"BaseDistanceCalculation2D");
    class_<BaseDistanceCalculationType3D, BaseDistanceCalculationType3D::Pointer, Process>(m,"BaseDistanceCalculation3D");

    class_< TrilinosVariationalDistanceCalculationProcess<2,TrilinosSparseSpaceType, TrilinosLocalSpaceType, TrilinosLinearSolverType>,
            BaseDistanceCalculationType2D >(m,"TrilinosVariationalDistanceCalculationProcess2D")
            .def(init<Epetra_MpiComm&, ModelPart&, TrilinosLinearSolverType::Pointer, unsigned int>() );

    class_< TrilinosVariationalDistanceCalculationProcess<3,TrilinosSparseSpaceType, TrilinosLocalSpaceType, TrilinosLinearSolverType>,
            BaseDistanceCalculationType3D >(m,"TrilinosVariationalDistanceCalculationProcess3D")
            .def(init<Epetra_MpiComm&, ModelPart&, TrilinosLinearSolverType::Pointer, unsigned int>() );
}

}
}

#endif
