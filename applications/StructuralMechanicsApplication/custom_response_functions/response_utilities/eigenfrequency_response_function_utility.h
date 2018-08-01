// KRATOS  ___|  |                   |                   |
//       \___ \  __|  __| |   |  __| __| |   |  __| _` | |
//             | |   |    |   | (    |   |   | |   (   | |
//       _____/ \__|_|   \__,_|\___|\__|\__,_|_|  \__,_|_| MECHANICS
//
//  License:         BSD License
//                     license: structural_mechanics_application/license.txt
//
//  Main authors:    Fusseder Martin, Armin Geiser, Daniel Baumgaertner
//

#ifndef EIGENFREQUENCY_RESPONSE_FUNCTION_UTILITY_H
#define EIGENFREQUENCY_RESPONSE_FUNCTION_UTILITY_H

// ------------------------------------------------------------------------------
// System includes
// ------------------------------------------------------------------------------
#include <iostream>
#include <string>

// ------------------------------------------------------------------------------
// External includes
// ------------------------------------------------------------------------------

// ------------------------------------------------------------------------------
// Project includes
// ------------------------------------------------------------------------------
#include "includes/define.h"
#include "includes/kratos_parameters.h"
#include "includes/model_part.h"
#include "utilities/variable_utils.h"

// ==============================================================================

namespace Kratos
{

///@name Kratos Globals
///@{

///@}
///@name Type Definitions
///@{

///@}
///@name  Enum's
///@{

///@}
///@name  Functions
///@{

///@}
///@name Kratos Classes
///@{

/// Short class definition.
/** Detail class definition.

 */

//template<class TDenseSpace>

class EigenfrequencyResponseFunctionUtility
{
public:
    ///@name Type Definitions
    ///@{

    /// Pointer definition of EigenfrequencyResponseFunctionUtility
    KRATOS_CLASS_POINTER_DEFINITION(EigenfrequencyResponseFunctionUtility);

    ///@}
    ///@name Life Cycle
    ///@{

    /// Default constructor.
    EigenfrequencyResponseFunctionUtility(ModelPart& model_part, Parameters response_settings)
    : mrModelPart(model_part)
    {
        CheckSettingsForGradientAnalysis(response_settings);
        DetermineTracedEigenfrequencies(response_settings);
        if(AreSeveralEigenfrequenciesTraced())
        {
            KRATOS_ERROR_IF_NOT(response_settings.Has("weighting_method")) << "Several eigenfrequencies are traced but no weighting method specified in the parameters!" << std::endl;
            if(response_settings["weighting_method"].GetString() == "linear_scaling")
                CalculateLinearWeights(response_settings);
            else
                KRATOS_ERROR  << "Specified weighting method for eigenfrequencies is not implemented! Available weighting methods are: 'linear_scaling'." << std::endl;
        }
        else
            UseDefaultWeight();
    }

    /// Destructor.
    virtual ~EigenfrequencyResponseFunctionUtility()
    {
    }

    ///@}
    ///@name Operators
    ///@{

    ///@}
    ///@name Operations
    ///@{

    // ==============================================================================
    void Initialize()
    {
        //not needed because only semi-analytical sensitivity analysis is implemented yet
    }

    // --------------------------------------------------------------------------
    double CalculateValue()
    {
        CheckIfAllNecessaryEigenvaluesAreComputed();

        double resp_function_value = 0.0;
        for(std::size_t i = 0; i < mTracedEigenfrequencyIds.size(); i++)
            resp_function_value += mWeightingFactors[i] * std::sqrt(GetEigenvalue(mTracedEigenfrequencyIds[i])) / (2*Globals::Pi);

        return resp_function_value;
    }

    // --------------------------------------------------------------------------
    void CalculateGradient()
    {
        CheckIfAllNecessaryEigenvaluesAreComputed();
        VariableUtils().SetToZero_VectorVar(SHAPE_SENSITIVITY, mrModelPart.Nodes());
        PerformSemiAnalyticSensitivityAnalysis();
    }

    // ==============================================================================

    ///@}
    ///@name Access
    ///@{

    ///@}
    ///@name Inquiry
    ///@{

    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    std::string Info() const
    {
        return "EigenfrequencyResponseFunctionUtility";
    }

    /// Print information about this object.
    virtual void PrintInfo(std::ostream &rOStream) const
    {
        rOStream << "EigenfrequencyResponseFunctionUtility";
    }

    /// Print object's data.
    virtual void PrintData(std::ostream &rOStream) const
    {
    }

    ///@}
    ///@name Friends
    ///@{

    ///@}

protected:
    ///@name Protected static Member Variables
    ///@{

    ///@}
    ///@name Protected member Variables
    ///@{

    ///@}
    ///@name Protected Operators
    ///@{

    ///@}
    ///@name Protected Operations
    ///@{

    // ==============================================================================
    void CheckSettingsForGradientAnalysis(Parameters rResponseSettings)
    {
        const std::string gradient_mode = rResponseSettings["gradient_mode"].GetString();

        if (gradient_mode == "semi_analytic")
            mDelta = rResponseSettings["step_size"].GetDouble();
        else
            KRATOS_ERROR << "Specified gradient_mode '" << gradient_mode << "' not recognized. The only option is: semi_analytic" << std::endl;
    }

    // --------------------------------------------------------------------------
    void DetermineTracedEigenfrequencies(Parameters rResponseSettings)
    {
        mTracedEigenfrequencyIds.resize(rResponseSettings["traced_eigenfrequencies"].size(),false);
        for(std::size_t i = 0; i < mTracedEigenfrequencyIds.size(); i++)
            mTracedEigenfrequencyIds[i] = rResponseSettings["traced_eigenfrequencies"][i].GetInt();
    }

    // --------------------------------------------------------------------------
    bool AreSeveralEigenfrequenciesTraced()
    {
        return (mTracedEigenfrequencyIds.size()>1);
    }

    // --------------------------------------------------------------------------
    void CalculateLinearWeights(Parameters rResponseSettings)
    {
        KRATOS_ERROR_IF_NOT(rResponseSettings.Has("weighting_factors")) << "No weighting factors defined for given eigenfrequency response!" << std::endl;
        KRATOS_ERROR_IF_NOT(rResponseSettings["weighting_factors"].size() == mTracedEigenfrequencyIds.size()) << "The number of chosen eigenvalues does not fit to the number of weighting factors!" << std::endl;

        mWeightingFactors.resize(rResponseSettings["weighting_factors"].size(),false);

        // Read weighting factors and sum them up
        double test_sum_weight_facs = 0.0;
        for(std::size_t i = 0; i < mWeightingFactors.size(); i++)
        {
            mWeightingFactors[i] = rResponseSettings["weighting_factors"][i].GetDouble();
            test_sum_weight_facs += mWeightingFactors[i];
        }

        // Check the weighting factors and modify them for the case that their sum is unequal to one
        if(std::abs(test_sum_weight_facs - 1.0) > 1e-12)
        {
            for(std::size_t i = 0; i < mWeightingFactors.size(); i++)
                mWeightingFactors[i] /= test_sum_weight_facs;

            KRATOS_INFO("\n> EigenfrequencyResponseFunctionUtility") << "The sum of the chosen weighting factors is unequal one. A corresponding scaling process was exected!" << std::endl;
        }
    }

    // --------------------------------------------------------------------------
    void UseDefaultWeight()
    {
        mWeightingFactors.resize(1,false);
        mWeightingFactors[0] = 1.0;
    }

    // --------------------------------------------------------------------------
    void CheckIfAllNecessaryEigenvaluesAreComputed()
    {
        const int num_of_computed_eigenvalues = (mrModelPart.GetProcessInfo()[EIGENVALUE_VECTOR]).size();
        const int max_required_eigenvalue = *(std::max_element(mTracedEigenfrequencyIds.begin(), mTracedEigenfrequencyIds.end()));
        KRATOS_ERROR_IF(max_required_eigenvalue > num_of_computed_eigenvalues) << "The following Eigenfrequency shall be traced but their corresponding eigenvalue was not computed by the eigenvalue analysies: " << max_required_eigenvalue << std::endl;
    }

    // --------------------------------------------------------------------------
    void PerformSemiAnalyticSensitivityAnalysis()
    {
        ProcessInfo &CurrentProcessInfo = mrModelPart.GetProcessInfo();

        // Predetermine all necessary eigenvalues and prefactors for gradient calculation
        const std::size_t num_of_traced_eigenfrequencies = mTracedEigenfrequencyIds.size();
        Vector traced_eigenvalues(num_of_traced_eigenfrequencies,0.0);
        Vector gradient_prefactors(num_of_traced_eigenfrequencies,0.0);
        for(std::size_t i = 0; i < num_of_traced_eigenfrequencies; i++)
        {
            traced_eigenvalues[i] = GetEigenvalue(mTracedEigenfrequencyIds[i]);
            gradient_prefactors[i] = 1 / (4 * Globals::Pi * std::sqrt(traced_eigenvalues[i]));
        }

        // Element-wise computation of gradients
        for (auto& elem_i : mrModelPart.Elements())
        {
            Matrix mass_matrix_org;
            Matrix LHS_org;
            Vector dummy;

            elem_i.CalculateMassMatrix(mass_matrix_org, CurrentProcessInfo);
            elem_i.CalculateLocalSystem(LHS_org, dummy ,CurrentProcessInfo);

            const std::size_t num_dofs_element = mass_matrix_org.size1();
            const std::size_t domain_size = CurrentProcessInfo.GetValue(DOMAIN_SIZE);

            Matrix aux_matrix = Matrix(num_dofs_element,num_dofs_element);
            Vector aux_vector = Vector(num_dofs_element);

            // Predetermine the necessary eigenvectors
            std::vector<Vector> eigenvectors_of_element(num_of_traced_eigenfrequencies,Vector(0));
            for(std::size_t i = 0; i < num_of_traced_eigenfrequencies; i++)
                DetermineEigenvectorOfElement(elem_i, mTracedEigenfrequencyIds[i], num_dofs_element, eigenvectors_of_element[i]);

            // Computation of derivative of state equation w.r.t. node coordinates
            for(auto& node_i : elem_i.GetGeometry())
            {
                Vector gradient_contribution(3, 0.0);
                Matrix perturbed_LHS = Matrix(num_dofs_element,num_dofs_element);
                Matrix perturbed_mass_matrix = Matrix(num_dofs_element,num_dofs_element);

                for(std::size_t coord_dir_i = 0; coord_dir_i < domain_size; coord_dir_i++)
                {
                    node_i.GetInitialPosition()[coord_dir_i] += mDelta;

                    elem_i.CalculateMassMatrix(perturbed_mass_matrix, CurrentProcessInfo);
                    noalias(perturbed_mass_matrix) = (perturbed_mass_matrix - mass_matrix_org) / mDelta;

                    elem_i.CalculateLocalSystem(perturbed_LHS, dummy ,CurrentProcessInfo);
                    noalias(perturbed_LHS) = (perturbed_LHS - LHS_org) / mDelta;

                    for(std::size_t i = 0; i < num_of_traced_eigenfrequencies; i++)
                    {
                        aux_matrix.clear();
                        aux_vector.clear();

                        noalias(aux_matrix) = perturbed_LHS - perturbed_mass_matrix * traced_eigenvalues[i];
                        noalias(aux_vector) = prod(aux_matrix , eigenvectors_of_element[i]);

                        gradient_contribution[coord_dir_i] += gradient_prefactors[i] * inner_prod(eigenvectors_of_element[i] , aux_vector) * mWeightingFactors[i];
                    }

                   node_i.GetInitialPosition()[coord_dir_i] -= mDelta;
                }
                noalias(node_i.FastGetSolutionStepValue(SHAPE_SENSITIVITY)) += gradient_contribution;
            }
        }
    }

    // --------------------------------------------------------------------------
    double GetEigenvalue(int eigenfrequency_id)
    {
        return (mrModelPart.GetProcessInfo()[EIGENVALUE_VECTOR])[eigenfrequency_id-1];
    }

    // --------------------------------------------------------------------------
    void DetermineEigenvectorOfElement(ModelPart::ElementType& traced_element, int eigenfrequency_id, int size_of_eigenvector, Vector& rEigenvectorOfElement)
    {
        rEigenvectorOfElement.resize(size_of_eigenvector,false);

        const std::size_t num_nodes = traced_element.GetGeometry().size();
        const std::size_t num_node_dofs = size_of_eigenvector/num_nodes;

        for (std::size_t node_index=0; node_index<num_nodes; node_index++)
        {
            Matrix& rNodeEigenvectors = traced_element.GetGeometry()[node_index].GetValue(EIGENVECTOR_MATRIX);

            for (std::size_t dof_index = 0; dof_index < num_node_dofs; dof_index++)
                rEigenvectorOfElement(dof_index+num_node_dofs*node_index) = rNodeEigenvectors((eigenfrequency_id-1),dof_index);
        }
    }

    // ==============================================================================

    ///@}
    ///@name Protected  Access
    ///@{

    ///@}
    ///@name Protected Inquiry
    ///@{

    ///@}
    ///@name Protected LifeCycle
    ///@{

    ///@}

private:
    ///@name Static Member Variables
    ///@{

    ///@}
    ///@name Member Variables
    ///@{

    ModelPart &mrModelPart;
    double mDelta;
    std::vector<int> mTracedEigenfrequencyIds;
    std::vector<double> mWeightingFactors;

    ///@}
///@name Private Operators
    ///@{

    ///@}
    ///@name Private Operations
    ///@{

    ///@}
    ///@name Private  Access
    ///@{

    ///@}
    ///@name Private Inquiry
    ///@{

    ///@}
    ///@name Un accessible methods
    ///@{

    /// Assignment operator.
    //      EigenfrequencyResponseFunctionUtility& operator=(EigenfrequencyResponseFunctionUtility const& rOther);

    /// Copy constructor.
    //      EigenfrequencyResponseFunctionUtility(EigenfrequencyResponseFunctionUtility const& rOther);

    ///@}

}; // Class EigenfrequencyResponseFunctionUtility

///@}

///@name Type Definitions
///@{

///@}
///@name Input and output
///@{

///@}

} // namespace Kratos.

#endif // EIGENFREQUENCY_RESPONSE_FUNCTION_UTILITY_H
