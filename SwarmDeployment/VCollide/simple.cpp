/************************************************************************\

  Copyright 1997 The University of North Carolina at Chapel Hill.
  All Rights Reserved.

  Permission to use, copy, modify and distribute this software
  and its documentation for educational, research and non-profit
  purposes, without fee, and without a written agreement is
  hereby granted, provided that the above copyright notice and
  the following three paragraphs appear in all copies.

  IN NO EVENT SHALL THE UNIVERSITY OF NORTH CAROLINA AT CHAPEL
  HILL BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL,
  INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
  ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
  EVEN IF THE UNIVERSITY OF NORTH CAROLINA HAVE BEEN ADVISED OF
  THE POSSIBILITY OF SUCH DAMAGES.


  Permission to use, copy, modify and distribute this software
  and its documentation for educational, research and non-profit
  purposes, without fee, and without a written agreement is
  hereby granted, provided that the above copyright notice and
  the following three paragraphs appear in all copies.

  THE UNIVERSITY OF NORTH CAROLINA SPECIFICALLY DISCLAIM ANY
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS"
  BASIS, AND THE UNIVERSITY OF NORTH CAROLINA HAS NO OBLIGATION
  TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
  MODIFICATIONS.


   --------------------------------- 
  |Please send all BUG REPORTS to:  |
  |                                 |
  |   geom@cs.unc.edu               |
  |                                 |
   ---------------------------------
  
     
  The authors may be contacted via:

  US Mail:  A. Pattekar/J. Cohen/T. Hudson/S. Gottschalk/M. Lin/D. Manocha
            Department of Computer Science
            Sitterson Hall, CB #3175
            University of N. Carolina
            Chapel Hill, NC 27599-3175
	    
  Phone:    (919)962-1749
	    
  EMail:    geom@cs.unc.edu

\************************************************************************/

#include <iostream>
#include <stdlib.h>
#include "VCollide.H"
#include "Triangle3D.h"
#include "ColDetect.h"
#include "Point2D.h"

int not_main2(int argc, char *argv[])
{
  
  if (argc != 1)
    {
	    std::cerr<<argv[0]<<": USAGE: "<<argv[0]<<"\n";
      exit(1);
    }
  
  VCollide vc;
  int id[2];

  int i;
  for (i=0; i<2; i++) //create both the objects.
    {
      vc.NewObject(&id[i]);
      
      //the geometry is a unit cube with one vertex at the origin.
      double v1[3], v2[3], v3[3];
      
      v1[0] = 0.0; v1[1] = 0.0; v1[2] = 0.0;
      v2[0] = 1.0; v2[1] = 0.0; v2[2] = 0.0;
      v3[0] = 1.0; v3[1] = 0.0; v3[2] = 1.0;
      vc.AddTri(v1, v2, v3, 0);
      
      v1[0] = 0.0; v1[1] = 0.0; v1[2] = 0.0;
      v2[0] = 0.0; v2[1] = 0.0; v2[2] = 1.0;
      v3[0] = 1.0; v3[1] = 0.0; v3[2] = 1.0;
      vc.AddTri(v1, v2, v3, 1);
      
      v1[0] = 0.0; v1[1] = 1.0; v1[2] = 0.0;
      v2[0] = 1.0; v2[1] = 1.0; v2[2] = 0.0;
      v3[0] = 1.0; v3[1] = 1.0; v3[2] = 1.0;
      vc.AddTri(v1, v2, v3, 2);
      
      v1[0] = 0.0; v1[1] = 1.0; v1[2] = 0.0;
      v2[0] = 0.0; v2[1] = 1.0; v2[2] = 1.0;
      v3[0] = 1.0; v3[1] = 1.0; v3[2] = 1.0;
      vc.AddTri(v1, v2, v3, 3);
      
      v1[0] = 1.0; v1[1] = 0.0; v1[2] = 0.0;
      v2[0] = 1.0; v2[1] = 1.0; v2[2] = 0.0;
      v3[0] = 1.0; v3[1] = 1.0; v3[2] = 1.0;
      vc.AddTri(v1, v2, v3, 4);

      v1[0] = 1.0; v1[1] = 0.0; v1[2] = 0.0;
      v2[0] = 1.0; v2[1] = 0.0; v2[2] = 1.0;
      v3[0] = 1.0; v3[1] = 1.0; v3[2] = 1.0;
      vc.AddTri(v1, v2, v3, 5);
      
      v1[0] = 0.0; v1[1] = 0.0; v1[2] = 0.0;
      v2[0] = 0.0; v2[1] = 1.0; v2[2] = 0.0;
      v3[0] = 0.0; v3[1] = 1.0; v3[2] = 1.0;
      vc.AddTri(v1, v2, v3, 6);
      
      v1[0] = 0.0; v1[1] = 0.0; v1[2] = 0.0;
      v2[0] = 0.0; v2[1] = 0.0; v2[2] = 1.0;
      v3[0] = 0.0; v3[1] = 1.0; v3[2] = 1.0;
      vc.AddTri(v1, v2, v3, 7);

      v1[0] = 1.0; v1[1] = 0.0; v1[2] = 1.0;
      v2[0] = 1.0; v2[1] = 1.0; v2[2] = 1.0;
      v3[0] = 0.0; v3[1] = 1.0; v3[2] = 1.0;
      vc.AddTri(v1, v2, v3, 8);

      v1[0] = 1.0; v1[1] = 0.0; v1[2] = 1.0;
      v2[0] = 0.0; v2[1] = 0.0; v2[2] = 1.0;
      v3[0] = 0.0; v3[1] = 1.0; v3[2] = 1.0;
      vc.AddTri(v1, v2, v3, 9);

      v1[0] = 1.0; v1[1] = 0.0; v1[2] = 0.0;
      v2[0] = 1.0; v2[1] = 1.0; v2[2] = 0.0;
      v3[0] = 0.0; v3[1] = 1.0; v3[2] = 0.0;
      vc.AddTri(v1, v2, v3, 10);

      v1[0] = 1.0; v1[1] = 0.0; v1[2] = 0.0;
      v2[0] = 0.0; v2[1] = 0.0; v2[2] = 0.0;
      v3[0] = 0.0; v3[1] = 1.0; v3[2] = 0.0;
      vc.AddTri(v1, v2, v3, 11);
      
      vc.EndObject();
    }

  double trans0[4][4], trans1[4][4]; //transformation matrices.

  //initialize the transformation matrices to identity.
  
  for (i=0; i<4; i++)
    {
      int j;
      for (j=0; j<4; j++)
	trans0[i][j] = trans1[i][j] = ( (i==j) ? 1.0 : 0.0 );
    }
  
  int simulation_step=1;
  for (i=-25; i<=25; i++) //perform 51 frames of the simulation
    {
	    std::cout<<"Simulation step: "<<simulation_step++<<"\n";
      
      //in successive frames of the simulation, the two objects 
      //approach each other from far and finally collide and cross
      //each other.
      trans0[0][3] =  0.25*i;  //we translate both the objects
      trans1[0][3] = -0.25*i;  //along the X-axis only.
      
      vc.UpdateTrans(id[0], trans0);
      vc.UpdateTrans(id[1], trans1);
  

      VCReport report;

      vc.Collide( &report );  //perform collision test.
                              //default is VC_FIRST_CONTACT
      
      int j;
      for (j = 0; j < report.numObjPairs(); j++)
      {
		  std::cout << "Detected collision between objects "
			  << report.obj1ID(j) << " and " << report.obj2ID(j) << "\n";

      }
      
    }
	std::cin.get();
  return 0;
}


int not_main3(int argc, char *argv[])
{
	int collision;
	ColDetect colDetect;
	Triangle3D* triangle1 = new Triangle3D(new Point2D(0, 0), new Point2D(2, 3), new Point2D(2, 0));
	Triangle3D* triangle2 = new Triangle3D(new Point2D(0, 1), new Point2D(1, 1), new Point2D(0, 3));
	collision = colDetect.coldetect(triangle1, triangle2);	//m� b�t kolize
	std::cout << "Collision: " << collision << std::endl;

	Triangle3D* triangle3 = new Triangle3D(new Point2D(0, 1), new Point2D(1, 2), new Point2D(0, 3));
	collision = colDetect.coldetect(triangle1, triangle3);	//nem� b�t kolize
	std::cout << "Collision: " << collision << std::endl;

	Triangle3D* triangle4 = new Triangle3D(new Point2D(0, 0), new Point2D(2, 2), new Point2D(2, 0));
	collision = colDetect.coldetect(triangle2, triangle4);	//maj� se dot�kat, je kolize
	std::cout << "Collision: " << collision << std::endl;

	Triangle3D* triangle5 = new Triangle3D(new Point2D(0, 0), new Point2D(2, 1.9999), new Point2D(2, 0));
	collision = colDetect.coldetect(triangle2, triangle5);	//nem� b�t kolize
	std::cout << "Collision: " << collision << std::endl;


	Rectangle2D* rect1 = new Rectangle2D(0, 0, 2, 2);
	Rectangle2D* rect2 = new Rectangle2D(1, 1, 2, 2);
	collision = colDetect.coldetect(rect1, rect2);	//nem� b�t kolize
	std::cout << "Collision: " << collision << std::endl;

	Rectangle2D* rect3 = new Rectangle2D(2, 1, 2, 2);
	collision = colDetect.coldetect(rect1, rect3);	//nem� b�t kolize
	std::cout << "Collision: " << collision << std::endl;

	Rectangle2D* rect4 = new Rectangle2D(3, 3, 3, 3);
	collision = colDetect.coldetect(rect1, rect4);	//nem� b�t kolize
	std::cout << "Collision: " << collision << std::endl;

	std::cin.get();
	return 0;
}