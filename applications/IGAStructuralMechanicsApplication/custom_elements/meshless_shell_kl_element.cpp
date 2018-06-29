//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/IGAStructuralMechanicsApplication/license.txt
//
//  Main authors:    Tobias Tescheamacher
//                   Riccardo Rossi
//


// System includes
#include "includes/define.h"
#include "utilities/math_utils.h"
#include "geometries/geometry.h"

// External includes


// Project includes
#include "custom_elements/meshless_base_element.h"
#include "custom_elements/meshless_base_surface_element.h"
#include "custom_elements/meshless_shell_kl_element.h"

// Application includes
#include "iga_structural_mechanics_application.h"
#include "iga_structural_mechanics_application_variables.h"

namespace Kratos
{
	//************************************************************************************
	//************************************************************************************
	void MeshlessShellKLElement::EquationIdVector(
		EquationIdVectorType& rResult,
		ProcessInfo& rCurrentProcessInfo
	)
	{
		KRATOS_TRY;
		unsigned int number_of_nodes = GetGeometry().size();
		unsigned int dim = number_of_nodes * 3;

		if (rResult.size() != dim)
			rResult.resize(dim);

		for (unsigned int i = 0; i < number_of_nodes; i++)
		{
			int index = i * 3;
			rResult[index]     = GetGeometry()[i].GetDof(DISPLACEMENT_X).EquationId();
			rResult[index + 1] = GetGeometry()[i].GetDof(DISPLACEMENT_Y).EquationId();
			rResult[index + 2] = GetGeometry()[i].GetDof(DISPLACEMENT_Z).EquationId();
		}
		KRATOS_CATCH("")
	};

	//************************************************************************************
	//************************************************************************************
	void MeshlessShellKLElement::GetDofList(
		DofsVectorType& rElementalDofList,
		ProcessInfo& rCurrentProcessInfo
	)
	{
		KRATOS_TRY;

		rElementalDofList.resize(0);

		for (unsigned int i = 0; i < GetGeometry().size(); i++)
		{
			rElementalDofList.push_back(GetGeometry()[i].pGetDof(DISPLACEMENT_X));
			rElementalDofList.push_back(GetGeometry()[i].pGetDof(DISPLACEMENT_Y));
			rElementalDofList.push_back(GetGeometry()[i].pGetDof(DISPLACEMENT_Z));
		}

		KRATOS_CATCH("")
	};
	//***********************************************************************************
	//***********************************************************************************
	void MeshlessShellKLElement::FinalizeSolutionStep(
		ProcessInfo& rCurrentProcessInfo)
	{
		mConstitutiveLaw->FinalizeSolutionStep(GetProperties(),
			GetGeometry(),
			this->GetValue(SHAPE_FUNCTION_VALUES),
			rCurrentProcessInfo);
	}
	//************************************************************************************
	//************************************************************************************
	void MeshlessShellKLElement::InitializeMaterial()
	{
		KRATOS_TRY

			if (GetProperties()[CONSTITUTIVE_LAW] != NULL)
			{
				//get shape functions for evaluation of stresses inside the constitutive law
				const Vector&  N = this->GetValue(SHAPE_FUNCTION_VALUES);
				const Matrix& DN_De = this->GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);
				const double integration_weight = this->GetValue(INTEGRATION_WEIGHT);
				
				mConstitutiveLaw = GetProperties()[CONSTITUTIVE_LAW]->Clone();
				ProcessInfo emptyProcessInfo = ProcessInfo();
				//mConstitutiveLaw->SetValue(INTEGRATION_WEIGHT, integration_weight, emptyProcessInfo);
				//mConstitutiveLaw->SetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES, DN_De, emptyProcessInfo);
				mConstitutiveLaw->InitializeMaterial(GetProperties(), GetGeometry(), N);
			}
			else
			{
				KRATOS_ERROR << "A constitutive law needs to be specified for the element with ID " << this->Id() << std::endl;
			}

		KRATOS_CATCH("");
	}
	//************************************************************************************
	//************************************************************************************
	void MeshlessShellKLElement::CalculateAll(
		MatrixType& rLeftHandSideMatrix,
		VectorType& rRightHandSideVector,
		ProcessInfo& rCurrentProcessInfo,
		const bool CalculateStiffnessMatrixFlag,
		const bool CalculateResidualVectorFlag
	)
	{
		KRATOS_TRY
		// definition of problem size
		const unsigned int number_of_nodes = GetGeometry().size();
		unsigned int mat_size = number_of_nodes * 3;

		//set up properties for Constitutive Law
		ConstitutiveLaw::Parameters Values(GetGeometry(), GetProperties(), rCurrentProcessInfo);

		Values.GetOptions().Set(ConstitutiveLaw::USE_ELEMENT_PROVIDED_STRAIN, true);
		Values.GetOptions().Set(ConstitutiveLaw::COMPUTE_STRESS);
		Values.GetOptions().Set(ConstitutiveLaw::COMPUTE_CONSTITUTIVE_TENSOR);

		//resizing as needed the LHS
		if (CalculateStiffnessMatrixFlag == true) //calculation of the matrix is required
		{
			if (rLeftHandSideMatrix.size1() != mat_size)
				rLeftHandSideMatrix.resize(mat_size, mat_size);
			noalias(rLeftHandSideMatrix) = ZeroMatrix(mat_size, mat_size); //resetting LHS
		}
		//resizing as needed the RHS
		if (CalculateResidualVectorFlag == true) //calculation of the matrix is required
		{
			if (rRightHandSideVector.size() != mat_size)
				rRightHandSideVector.resize(mat_size);
			rRightHandSideVector = ZeroVector(mat_size); //resetting RHS
		}

		//reading in of integration weight, shape function values and shape function derivatives
		double integration_weight = this->GetValue(INTEGRATION_WEIGHT);
		Vector   N     = this->GetValue(SHAPE_FUNCTION_VALUES);
		Matrix  DN_De  = this->GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);
		Matrix DDN_DDe = this->GetValue(SHAPE_FUNCTION_LOCAL_SECOND_DERIVATIVES);

		MetricVariables actual_metric(3);
		CalculateMetric(actual_metric);
		ConstitutiveVariables constitutive_variables(3);
		CalculateConstitutiveVariables(actual_metric, constitutive_variables, Values, ConstitutiveLaw::StressMeasure_PK2);


		// calculate B MATRICES
		Matrix BMembrane = ZeroMatrix(3, mat_size);
		Matrix BCurvature = ZeroMatrix(3, mat_size);
		CalculateBMembrane(BMembrane, actual_metric);
		CalculateBCurvature(BCurvature, actual_metric);

		// Nonlinear Deformation
		Matrix Strain_ca_11 = ZeroMatrix(number_of_nodes * 3, number_of_nodes * 3);
		Matrix Strain_ca_22 = ZeroMatrix(number_of_nodes * 3, number_of_nodes * 3);
		Matrix Strain_ca_12 = ZeroMatrix(number_of_nodes * 3, number_of_nodes * 3);
		Matrix Curvature_ca_11 = ZeroMatrix(number_of_nodes * 3, number_of_nodes * 3);
		Matrix Curvature_ca_22 = ZeroMatrix(number_of_nodes * 3, number_of_nodes * 3);
		Matrix Curvature_ca_12 = ZeroMatrix(number_of_nodes * 3, number_of_nodes * 3);
		CalculateSecondVariationStrainCurvature(
			Strain_ca_11, Strain_ca_22, Strain_ca_12,
			Curvature_ca_11, Curvature_ca_22, Curvature_ca_12, actual_metric);

		integration_weight = this->GetValue(INTEGRATION_WEIGHT) * mInitialMetric.detJ * GetProperties()[THICKNESS];

		// LEFT HAND SIDE MATRIX
		if (CalculateStiffnessMatrixFlag == true)
		{
			//adding membrane contributions to the stiffness matrix
			CalculateAndAddKm(rLeftHandSideMatrix, BMembrane, constitutive_variables.DMembrane, integration_weight);
			//adding curvature contributions to the stiffness matrix
			CalculateAndAddKm(rLeftHandSideMatrix, BCurvature, constitutive_variables.DCurvature, integration_weight);

			// adding  non-linear-contribution to Stiffness-Matrix
			CalculateAndAddNonlinearKm(rLeftHandSideMatrix,
				Strain_ca_11, Strain_ca_22, Strain_ca_12,
				constitutive_variables.StressVector,
				integration_weight);

			CalculateAndAddNonlinearKm(rLeftHandSideMatrix,
				Curvature_ca_11, Curvature_ca_22, Curvature_ca_12,
				constitutive_variables.StressCurvatureVector,
				integration_weight);
		}
		//KRATOS_WATCH(rLeftHandSideMatrix)

		// RIGHT HAND SIDE VECTOR
		if (CalculateResidualVectorFlag == true) //calculation of the matrix is required
		{
			// operation performed: rRightHandSideVector -= Weight*IntForce
			noalias(rRightHandSideVector) -= integration_weight * prod(trans(BMembrane), constitutive_variables.StressVector);
			noalias(rRightHandSideVector) += integration_weight * prod(trans(BCurvature), constitutive_variables.StressCurvatureVector);
		}
		KRATOS_CATCH("");
	}

	//************************************************************************************
	//************************************************************************************
	void MeshlessShellKLElement::CalculateOnIntegrationPoints(
		const Variable<double>& rVariable,
		std::vector<double>& rOutput,
		const ProcessInfo& rCurrentProcessInfo
	)
	{
		if (rOutput.size() != 1)
			rOutput.resize(1);

		if (rVariable == VON_MISES_STRESS)
		{
			// Create constitutive law parameters:
			ConstitutiveLaw::Parameters Values(GetGeometry(), GetProperties(), rCurrentProcessInfo);
			// Set constitutive law flags:
			Flags& ConstitutiveLawOptions = Values.GetOptions();
			ConstitutiveLawOptions.Set(ConstitutiveLaw::USE_ELEMENT_PROVIDED_STRAIN, true);
			ConstitutiveLawOptions.Set(ConstitutiveLaw::COMPUTE_STRESS, true);
			ConstitutiveLawOptions.Set(ConstitutiveLaw::COMPUTE_CONSTITUTIVE_TENSOR, true);

			MetricVariables actual_metric(3);
			CalculateMetric(actual_metric);
			ConstitutiveVariables constitutive_variables(3);
			CalculateConstitutiveVariables(actual_metric, constitutive_variables, Values, ConstitutiveLaw::StressMeasure_PK2);

			double detF = actual_metric.dA / mInitialMetric.dA;

			Vector n_pk2_ca = prod(constitutive_variables.DMembrane, constitutive_variables.StrainVector);
			Vector n_pk2_con = prod(mInitialMetric.T, n_pk2_ca);
			Vector n_cau = 1.0 / detF*n_pk2_con;

			Vector n = ZeroVector(8);
			// Cauchy normal force in normalized g1,g2
			n[0] = sqrt(actual_metric.gab[0] / actual_metric.gab_con[0])*n_cau[0];
			n[1] = sqrt(actual_metric.gab[1] / actual_metric.gab_con[1])*n_cau[1];
			n[2] = sqrt(actual_metric.gab[0] / actual_metric.gab_con[1])*n_cau[2];
			// Cauchy normal force in local cartesian e1,e2
			array_1d<double, 3> n_e = prod(actual_metric.T, n_cau);
			n[3] = n_e[0];
			n[4] = n_e[1];
			n[5] = n_e[2];
			// Principal normal forces
			n[6] = 0.5*(n_e[0] + n_e[1] + sqrt(pow(n_e[0] - n_e[1], 2) + 4.0*pow(n_e[2], 2)));
			n[7] = 0.5*(n_e[0] + n_e[1] - sqrt(pow(n_e[0] - n_e[1], 2) + 4.0*pow(n_e[2], 2)));

			// -------------------  moments -------------------------
			// PK2 moment in local cartesian E1,E2
			Vector m_pk2_local_ca = prod(constitutive_variables.DCurvature, constitutive_variables.StrainCurvatureVector);
			// PK2 moment in G1,G2
			Vector m_pk2_con = prod(mInitialMetric.T, m_pk2_local_ca);
			// Cauchy moment in g1,g2
			array_1d<double, 3> m_cau = 1.0 / detF*m_pk2_con; 


			Vector m = ZeroVector(8);
			// Cauchy moment in normalized g1,g2
			m[0] = sqrt(actual_metric.gab[0] / actual_metric.gab_con[0])*m_cau[0];
			m[1] = sqrt(actual_metric.gab[1] / actual_metric.gab_con[1])*m_cau[1];
			m[2] = sqrt(actual_metric.gab[0] / actual_metric.gab_con[1])*m_cau[2];
			// Cauchy moment in local cartesian e1,e2
			Vector m_e = prod(actual_metric.T, m_cau);
			m[3] = m_e[0];
			m[4] = m_e[1];
			m[5] = m_e[2];
			// principal moment
			m[6] = 0.5*(m_e[0] + m_e[1] + sqrt(pow(m_e[0] - m_e[1], 2) + 4.0*pow(m_e[2], 2)));
			m[7] = 0.5*(m_e[0] + m_e[1] - sqrt(pow(m_e[0] - m_e[1], 2) + 4.0*pow(m_e[2], 2)));

			double thickness = this->GetProperties().GetValue(THICKNESS);

			double W = pow(thickness, 2) / 6.0;
			double sigma_1_top = m[3] / W + n[3] / thickness;
			double sigma_2_top = m[4] / W + n[4] / thickness;
			double sigma_3_top = m[5] / W + n[5] / thickness;
			double vMises = pow(pow(sigma_1_top, 2) + pow(sigma_2_top, 2) - sigma_1_top*sigma_2_top + 3 * pow(sigma_3_top, 2), 0.5);

			rOutput[0] = vMises;
		}
		else if (rVariable == DAMAGE_T)
		{
			mConstitutiveLaw->GetValue(DAMAGE_T, rOutput[0]);
		}
		else if (rVariable == DAMAGE_C)
		{
			mConstitutiveLaw->GetValue(DAMAGE_C, rOutput[0]);
		}
		else
		{
			rOutput[0] = 0.0;// mConstitutiveLawVector[0]->GetValue(rVariable, rOutput[0]);
		}
	}

	void MeshlessShellKLElement::CalculateOnIntegrationPoints(
		const Variable<Vector>& rVariable,
		std::vector<Vector>& rValues,
		const ProcessInfo& rCurrentProcessInfo
	)
	{
		if (rValues.size() != 1)
		{
			rValues.resize(1);
		}

		if (rVariable == STRESSES)
		{
			// Create constitutive law parameters:
			ConstitutiveLaw::Parameters Values(GetGeometry(), GetProperties(), rCurrentProcessInfo);
			// Set constitutive law flags:
			Flags& ConstitutiveLawOptions = Values.GetOptions();
			ConstitutiveLawOptions.Set(ConstitutiveLaw::USE_ELEMENT_PROVIDED_STRAIN, true);
			ConstitutiveLawOptions.Set(ConstitutiveLaw::COMPUTE_STRESS, true);
			ConstitutiveLawOptions.Set(ConstitutiveLaw::COMPUTE_CONSTITUTIVE_TENSOR, true);

			MetricVariables actual_metric(3);
			CalculateMetric(actual_metric);
			ConstitutiveVariables constitutive_variables(3);
			CalculateConstitutiveVariables(actual_metric, constitutive_variables, Values, ConstitutiveLaw::StressMeasure_PK2);

			double detF = actual_metric.dA / mInitialMetric.dA;

			Vector n_pk2_ca = prod(constitutive_variables.DMembrane, constitutive_variables.StrainVector);
			Vector n_pk2_con = prod(mInitialMetric.T, n_pk2_ca);
			Vector n_cau = 1.0 / detF*n_pk2_con;

			Vector n = ZeroVector(8);
			// Cauchy normal force in normalized g1,g2
			n[0] = sqrt(actual_metric.gab[0] / actual_metric.gab_con[0])*n_cau[0];
			n[1] = sqrt(actual_metric.gab[1] / actual_metric.gab_con[1])*n_cau[1];
			n[2] = sqrt(actual_metric.gab[0] / actual_metric.gab_con[1])*n_cau[2];
			// Cauchy normal force in local cartesian e1,e2
			array_1d<double, 3> n_e = prod(actual_metric.T, n_cau);
			n[3] = n_e[0];
			n[4] = n_e[1];
			n[5] = n_e[2];
			// Principal normal forces
			n[6] = 0.5*(n_e[0] + n_e[1] + sqrt(pow(n_e[0] - n_e[1], 2) + 4.0*pow(n_e[2], 2)));
			n[7] = 0.5*(n_e[0] + n_e[1] - sqrt(pow(n_e[0] - n_e[1], 2) + 4.0*pow(n_e[2], 2)));

			// -------------------  moments -------------------------
			// PK2 moment in local cartesian E1,E2
			Vector m_pk2_local_ca = prod(constitutive_variables.DCurvature, constitutive_variables.StrainCurvatureVector);
			// PK2 moment in G1,G2
			Vector m_pk2_con = prod(mInitialMetric.T, m_pk2_local_ca);
			// Cauchy moment in g1,g2
			array_1d<double, 3> m_cau = 1.0 / detF*m_pk2_con;


			Vector m = ZeroVector(8);
			// Cauchy moment in normalized g1,g2
			m[0] = sqrt(actual_metric.gab[0] / actual_metric.gab_con[0])*m_cau[0];
			m[1] = sqrt(actual_metric.gab[1] / actual_metric.gab_con[1])*m_cau[1];
			m[2] = sqrt(actual_metric.gab[0] / actual_metric.gab_con[1])*m_cau[2];
			// Cauchy moment in local cartesian e1,e2
			Vector m_e = prod(actual_metric.T, m_cau);
			m[3] = m_e[0];
			m[4] = m_e[1];
			m[5] = m_e[2];
			// principal moment
			m[6] = 0.5*(m_e[0] + m_e[1] + sqrt(pow(m_e[0] - m_e[1], 2) + 4.0*pow(m_e[2], 2)));
			m[7] = 0.5*(m_e[0] + m_e[1] - sqrt(pow(m_e[0] - m_e[1], 2) + 4.0*pow(m_e[2], 2)));

			double thickness = this->GetProperties().GetValue(THICKNESS);

			double W = pow(thickness, 2) / 6.0;
			Vector sigma_top = ZeroVector(3);
			sigma_top(0) = m[3] / W + n[3] / thickness;
			sigma_top(1) = m[4] / W + n[4] / thickness;
			sigma_top(2) = m[5] / W + n[5] / thickness;

			rValues[0] = sigma_top;
		}
		else if (rVariable == EXTERNAL_FORCES_VECTOR) {
			const int& number_of_nodes = GetGeometry().size();
			Vector N = this->GetValue(SHAPE_FUNCTION_VALUES);
			Vector external_forces_vector = ZeroVector(3);
			for (SizeType i = 0; i < number_of_nodes; i++)
			{
				const NodeType & iNode = GetGeometry()[i];
				const Vector& forces = iNode.GetValue(EXTERNAL_FORCES_VECTOR);

				external_forces_vector[0] += N[i] * forces[0];
				external_forces_vector[1] += N[i] * forces[1];
				external_forces_vector[2] += N[i] * forces[2];
			}
			rValues[0] = external_forces_vector;
		}
		else
		{
			rValues[0] = ZeroVector(3);
		}
	}

	//************************************************************************************
	//************************************************************************************
	void MeshlessShellKLElement::CalculateMetric(
		MetricVariables& metric
	)
	{
		const Matrix& DN_De = this->GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);
		const Matrix& DDN_DDe = this->GetValue(SHAPE_FUNCTION_LOCAL_SECOND_DERIVATIVES);

		Jacobian(DN_De, metric.J);


		metric.g1[0] = metric.J(0, 0);
		metric.g2[0] = metric.J(0, 1);
		metric.g1[1] = metric.J(1, 0);
		metric.g2[1] = metric.J(1, 1);
		metric.g1[2] = metric.J(2, 0);
		metric.g2[2] = metric.J(2, 1);

		//basis vector g3
		MathUtils<double>::CrossProduct(metric.g3, metric.g1, metric.g2);
		//differential area dA
		metric.dA = norm_2(metric.g3);
		//normal vector _n
		Vector n = metric.g3 / metric.dA;


		//GetCovariantMetric
		metric.gab[0] = pow(metric.g1[0], 2) + pow(metric.g1[1], 2) + pow(metric.g1[2], 2);
		metric.gab[1] = pow(metric.g2[0], 2) + pow(metric.g2[1], 2) + pow(metric.g2[2], 2);
		metric.gab[2] = metric.g1[0] * metric.g2[0] + metric.g1[1] * metric.g2[1] + metric.g1[2] * metric.g2[2];

		Hessian(metric.H, DDN_DDe);

		metric.curvature[0] = metric.H(0, 0)*n[0] + metric.H(1, 0)*n[1] + metric.H(2, 0)*n[2];
		metric.curvature[1] = metric.H(0, 1)*n[0] + metric.H(1, 1)*n[1] + metric.H(2, 1)*n[2];
		metric.curvature[2] = metric.H(0, 2)*n[0] + metric.H(1, 2)*n[1] + metric.H(2, 2)*n[2];


		//contravariant metric gab_con and base vectors g_con
		//Vector gab_con = ZeroVector(3);
		double invdetGab = 1.0 / (metric.gab[0] * metric.gab[1] - metric.gab[2] * metric.gab[2]);
		metric.gab_con[0] = invdetGab*metric.gab[1];
		metric.gab_con[2] = -invdetGab*metric.gab[2];
		metric.gab_con[1] = invdetGab*metric.gab[0];


		array_1d<double, 3> g_con_1 = metric.g1*metric.gab_con[0] + metric.g2*metric.gab_con[2];
		array_1d<double, 3> g_con_2 = metric.g1*metric.gab_con[2] + metric.g2*metric.gab_con[1];


		//local cartesian coordinates
		double lg1 = norm_2(metric.g1);
		array_1d<double, 3> e1 = metric.g1 / lg1;
		double lg_con2 = norm_2(g_con_2);
		array_1d<double, 3> e2 = g_con_2 / lg_con2;

		//Matrix T_G_E = ZeroMatrix(3, 3);
		//Transformation matrix T from contravariant to local cartesian basis
		double eG11 = inner_prod(e1, metric.g1);
		double eG12 = inner_prod(e1, metric.g2);
		double eG21 = inner_prod(e2, metric.g1);
		double eG22 = inner_prod(e2, metric.g2);

		metric.Q = ZeroMatrix(3, 3);
		metric.Q(0, 0) = eG11*eG11;
		metric.Q(0, 1) = eG12*eG12;
		metric.Q(0, 2) = 2.0*eG11*eG12;
		metric.Q(1, 0) = eG21*eG21;
		metric.Q(1, 1) = eG22*eG22;
		metric.Q(1, 2) = 2.0*eG21*eG22;
		metric.Q(2, 0) = 2.0*eG11*eG21;
		metric.Q(2, 1) = 2.0*eG12*eG22;
		metric.Q(2, 2) = 2.0*eG11*eG22 + eG12*eG21;

		metric.T = ZeroMatrix(3, 3);
		metric.T(0, 0) = eG11*eG11;
		metric.T(0, 1) = eG21*eG21;
		metric.T(0, 2) = 2.0*eG11*eG21;
		metric.T(1, 0) = eG12*eG12;
		metric.T(1, 1) = eG22*eG22;
		metric.T(1, 2) = 2.0*eG12*eG22;
		metric.T(2, 0) = eG11*eG12;
		metric.T(2, 1) = eG21*eG22;
		metric.T(2, 2) = eG11*eG22 + eG12*eG21;
	}
	//************************************************************************************
	//************************************************************************************
	void MeshlessShellKLElement::CalculateConstitutiveVariables(
		MetricVariables& rActualMetric,
		ConstitutiveVariables& rThisConstitutiveVariables,
		ConstitutiveLaw::Parameters& rValues,
		const ConstitutiveLaw::StressMeasure ThisStressMeasure
	)
	{
		Vector strain_vector = ZeroVector(3);
		Vector curvature_vector = ZeroVector(3);

		CalculateStrain(strain_vector, rActualMetric.gab, mInitialMetric.gab);
		rThisConstitutiveVariables.StrainVector = prod(mInitialMetric.Q, strain_vector);
		CalculateCurvature(curvature_vector, rActualMetric.curvature, mInitialMetric.curvature);
		rThisConstitutiveVariables.StrainCurvatureVector = prod(mInitialMetric.Q, curvature_vector);

		//Constitive Matrices DMembrane and DCurvature
		rValues.SetStrainVector(rThisConstitutiveVariables.StrainVector); //this is the input parameter
		rValues.SetStressVector(rThisConstitutiveVariables.StressVector);    //this is an ouput parameter
		rValues.SetConstitutiveMatrix(rThisConstitutiveVariables.DMembrane); //this is an ouput parameter
		//rValues.CheckAllParameters();
		mConstitutiveLaw->CalculateMaterialResponse(rValues, ThisStressMeasure);
		
		double thickness = this->GetProperties().GetValue(THICKNESS);
		rThisConstitutiveVariables.DCurvature = rThisConstitutiveVariables.DMembrane*(pow(thickness, 2) / 12);

		//Local Cartesian Forces and Moments
		rThisConstitutiveVariables.StressVector = prod(
			trans(rThisConstitutiveVariables.DMembrane), rThisConstitutiveVariables.StrainVector);
		rThisConstitutiveVariables.StressCurvatureVector = prod(
			trans(rThisConstitutiveVariables.DCurvature), rThisConstitutiveVariables.StrainCurvatureVector);
	}

	void MeshlessShellKLElement::CalculateMassMatrix(
		MatrixType& rMassMatrix,
		ProcessInfo& rCurrentProcessInfo
	)
	{
		KRATOS_TRY;

		double integration_weight = this->GetValue(INTEGRATION_WEIGHT);
		Vector ShapeFunctionsN = this->GetValue(SHAPE_FUNCTION_VALUES);
		Matrix DN_De = this->GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);

		double density = this->GetProperties().GetValue(DENSITY);


		Vector g1, g2, g3;

		Matrix J;
		Jacobian(DN_De, J);

		g1[0] = J(0, 0);
		g2[0] = J(0, 1);
		g1[1] = J(1, 0);
		g2[1] = J(1, 1);
		g1[2] = J(2, 0);
		g2[2] = J(2, 1);

		CrossProduct2(g3, g1, g2);
		//differential area dA
		double dA = norm_2(g3);
		KRATOS_WATCH(dA)

			unsigned int dimension = 3;
		unsigned int number_of_nodes = ShapeFunctionsN.size();
		unsigned int mat_size = dimension * number_of_nodes;

		if (rMassMatrix.size1() != mat_size)
		{
			rMassMatrix.resize(mat_size, mat_size, false);
		}
		rMassMatrix = ZeroMatrix(mat_size, mat_size);

		for (int r = 0; r<number_of_nodes; r++)
		{
			for (int s = 0; s<number_of_nodes; s++)
			{
				rMassMatrix(3 * s, 3 * r) = ShapeFunctionsN(s)*ShapeFunctionsN(r) * density * dA * integration_weight;
				rMassMatrix(3 * s + 1, 3 * r + 1) = rMassMatrix(3 * s, 3 * r);
				rMassMatrix(3 * s + 2, 3 * r + 2) = rMassMatrix(3 * s, 3 * r);
			}
		}

		KRATOS_CATCH("")
	}

	//************************************************************************************
	//************************************************************************************
	int MeshlessShellKLElement::Check(const ProcessInfo& rCurrentProcessInfo)
	{
		KRATOS_TRY;
		if (DISPLACEMENT.Key() == 0)
			KRATOS_ERROR << "DISPLACEMENT has Key zero! check if the application is correctly registered" << std::endl;
		if (SHAPE_FUNCTION_VALUES.Key() == 0)
			KRATOS_ERROR << "SHAPE_FUNCTION_VALUES has Key zero! check if the application is correctly registered" << std::endl;
		if (SHAPE_FUNCTION_LOCAL_DERIVATIVES.Key() == 0)
			KRATOS_ERROR << "SHAPE_FUNCTION_LOCAL_DERIVATIVES has Key zero! check if the application is correctly registered" << std::endl;
		if (DISPLACEMENT.Key() == 0)
			KRATOS_ERROR << "SHAPE_FUNCTION_LOCAL_SECOND_DERIVATIVES has Key zero! check if the application is correctly registered" << std::endl;
		return 0;
		KRATOS_CATCH("");
	}


} // Namespace Kratos

