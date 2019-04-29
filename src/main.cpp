/******************************************************************************
 *                                                                            *
 * Copyright (C) 2018 Fondazione Istituto Italiano di Tecnologia (IIT)        *
 * All Rights Reserved.                                                       *
 *                                                                            *
 ******************************************************************************/

/**
 * @file main.cpp
 * @authors: Giulia Vezzani <giulia.vezzani@iit.it>
 */

 #include <cstdlib>
 #include <string>
 #include <vector>
 #include <cmath>
 #include <algorithm>
 #include <memory>

 #include <yarp/os/all.h>
 #include <yarp/sig/all.h>
 #include <yarp/math/Math.h>

 #include "src/TestLocalizer_IDL.h"

 using namespace std;

 using namespace yarp::os;
 using namespace yarp::sig;

/****************************************************************/
class TestLocalizeSuperq : public RFModule, TestLocalizer_IDL
{
    string moduleName;

    RpcClient point_cloud_rpc;
    RpcClient superq_rpc;

    RpcServer user_rpc;

    string object_name;

    PointCloudXYZRGBA point_cloud;

    vector<Vector> superqs;

  /****************************************************************/
  bool configure(ResourceFinder &rf) override
  {
       moduleName = rf.check("name", Value("test-localize-superq")).toString();

       //  open the necessary ports
       point_cloud_rpc.open("/" + moduleName + "/pointCloud:rpc");
       superq_rpc.open("/" + moduleName + "/localizeSuperq:rpc");
       user_rpc.open("/" + moduleName + "/cmd:rpc");

       //  attach callback
       attach(user_rpc);
  }

  /****************************************************************/
  bool compute_multiple_superqs(const vector<string> &objects)
  {
      double t0=Time::now();
      for (size_t i=0; i<objects.size(); i++)
      {
          askPointCloud(objects[i]);

          Bottle cmd, reply;
          Vector superquadric(11,0.0);

          cmd.addString("localize_superq");
          Bottle &list_objs=cmd.addList();
          for (size_t i=0; i< objects.size(); i++)
          {
              list_objs.addString(objects[i]);
          }

          Bottle &list_points=cmd.addList();

          for (size_t i=0; i< point_cloud.size(); i++)
          {
              Bottle &point=list_points.addList();
              point.addDouble(point_cloud(i).x);
              point.addDouble(point_cloud(i).y);
              point.addDouble(point_cloud(i).z);
          }

          superquadric=superq_rpc.write(cmd, reply);

          superqs.push_back(superquadric);
      }

      double t1=Time::now();

      yInfo() << "Total time for modeling" << objects.size() << "objects is" << t1 - t0;
  }

  /****************************************************************/
  bool askPointCloud(const string &object_name)
  {
      yarp::sig::PointCloud<DataXYZRGBA> pc;

      Bottle cmd_reply, cmd_request;

      cmd_request.addString("get_point_cloud");
      cmd_request.addString(object_name);

      if(point_cloud_rpc.getOutputCount() < 1)
      {
          yError() << "askPointCloud: no connection to point cloud module";
          return false;
      }
      point_cloud_rpc.write(cmd_request, cmd_reply);

      Bottle* pcBt = cmd_reply.get(0).asList();
      bool success = point_cloud.fromBottle(*pcBt);

      if (success)
        return true;
  }

  /****************************************************************/
    bool updateModule() override
    {
        return true;
    }

    /****************************************************************/
    double getPeriod() override
    {
        return 0.1;
    }

    /****************************************************************/
    bool interruptModule() override
    {
        point_cloud_rpc.interrupt();
        user_rpc.interrupt();
        superq_rpc.interrupt();

        return true;
    }

    /****************************************************************/
    bool close() override
    {
        point_cloud_rpc.close();
        user_rpc.close();
        superq_rpc.close();

        return true;
    }

    /************************************************************************/
    bool attach(RpcServer &source)
    {
      return this->yarp().attachAsServer(source);
    }
};

int main(int argc, char *argv[])
{
    Network yarp;
    ResourceFinder rf;
    rf.configure(argc, argv);

    if (!yarp.checkNetwork())
    {
        yError() << "YARP network not detected. Check nameserver";
        return EXIT_FAILURE;
    }

    TestLocalizeSuperq disp;

    return disp.runModule(rf);
}
