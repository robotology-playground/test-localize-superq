/******************************************************************************
 *                                                                            *
 * Copyright (C) 2018 Fondazione Istituto Italiano di Tecnologia (IIT)        *
 * All Rights Reserved.                                                       *
 *                                                                            *
 ******************************************************************************/

/**
 * @file idl.thirft
 * @authors: Giulia Vezzani <giulia.vezzani@iit.it>
 */


 service TestLocalizer_IDL
 {
	 bool compute_multiple_superqs(1: list<string> &object_names);
 }
