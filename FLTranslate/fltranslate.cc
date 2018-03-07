// Standard Library
#include <exception>
#include <iostream>
#include <string>
#include <vector>

// Third Party
// - Boost
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/filesystem.hpp"
// ROOT
#include "TFile.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
// CL options
#include "getopt_pp.h"

// - Bayeux
#include "bayeux/bayeux.h"
#include "bayeux/datatools/configuration/variant_service.h"
#include "bayeux/datatools/kernel.h"
#include "bayeux/datatools/multi_properties.h"
#include "bayeux/datatools/service_manager.h"
#include "bayeux/datatools/things.h"
#include "bayeux/datatools/urn.h"
#include "bayeux/datatools/urn_db_service.h"
#include "bayeux/datatools/urn_query_service.h"
#include "bayeux/datatools/urn_to_path_resolver_service.h"
#include "bayeux/dpp/output_module.h"
#include "bayeux/geomtools/geom_id.h"
#include "bayeux/version.h"

// This Project
#include <falaise/snemo/datamodels/calibrated_data.h>
#include <falaise/snemo/processing/calorimeter_regime.h>
#include "FLSimulateArgs.h"
#include "falaise/exitcodes.h"
#include "falaise/falaise.h"
#include "falaise/resource.h"
#include "falaise/snemo/datamodels/data_model.h"
#include "falaise/snemo/processing/services.h"
#include "falaise/version.h"

void showHelp() {
  std::cout << "fltranslate command line option(s) help" << std::endl;
  std::cout << "\t -i , --inputHitsFile <FULL PATH ROOT FILENAME>" << std::endl;
  std::cout << "\t -s , --start <First events>" << std::endl;
  std::cout << "\t -e , --end <Final events>" << std::endl;
  std::cout << "\t -o , --outputFile <FULL PATH BRIO FILENAME>" << std::endl;
}

typedef struct {
} validation_struct;

namespace FLTranslate {

//! Perform translation using command line args as given
falaise::exit_code do_fltranslate(int argc, char* argv[]);

//! Populate the metadata container with various informations classified in
//! several categories
falaise::exit_code do_metadata(const FLSimulate::FLSimulateArgs&,
                               datatools::multi_properties&);

}  // end of namespace FLTranslate

//----------------------------------------------------------------------
// MAIN PROGRAM
//----------------------------------------------------------------------
int main(int argc, char* argv[]) {
  falaise::initialize();

  // - Do the simulation.
  // Ideally, exceptions SHOULD NOT propagate out of this  - the error
  // code should be enough.
  falaise::exit_code ret = FLTranslate::do_fltranslate(argc, argv);

  falaise::terminate();
  return ret;
}

namespace FLTranslate {

//----------------------------------------------------------------------
falaise::exit_code do_metadata(
    const FLSimulate::FLSimulateArgs& flSimParameters,
    datatools::multi_properties& flSimMetadata) {
  falaise::exit_code code = falaise::EXIT_OK;

  // System section:
  datatools::properties& system_props =
      flSimMetadata.add_section("flsimulate", "flsimulate::section");
  system_props.set_description("flsimulate basic system informations");

  system_props.store_string("bayeux.version", bayeux::version::get_version(),
                            "Bayeux version");

  system_props.store_string("falaise.version", falaise::version::get_version(),
                            "Falaise version");

  system_props.store_string(
      "application", "flsimulate",
      "The simulation application used to produce Monte Carlo data");

  system_props.store_string("application.version",
                            falaise::version::get_version(),
                            "The version of the simulation application");

  system_props.store_string("userProfile", flSimParameters.userProfile,
                            "User profile");

  system_props.store_integer("numberOfEvents", flSimParameters.numberOfEvents,
                             "Number of simulated events");

  system_props.store_boolean("doSimulation", flSimParameters.doSimulation,
                             "Activate simulation");

  system_props.store_boolean("doDigitization", flSimParameters.doDigitization,
                             "Activate digitization");

  if (!flSimParameters.experimentalSetupUrn.empty()) {
    system_props.store_string("experimentalSetupUrn",
                              flSimParameters.experimentalSetupUrn,
                              "Experimental setup URN");
  }

  system_props.store_boolean("embeddedMetadata",
                             flSimParameters.embeddedMetadata,
                             "Metadata embedding flag");

  boost::posix_time::ptime start_run_timestamp =
      boost::posix_time::second_clock::universal_time();
  system_props.store_string(
      "timestamp", boost::posix_time::to_iso_string(start_run_timestamp),
      "Run start timestamp");

  if (flSimParameters.doSimulation) {
    // Simulation section:
    datatools::properties& simulation_props = flSimMetadata.add_section(
        "flsimulate.simulation", "flsimulate::section");
    simulation_props.set_description("Simulation setup parameters");

    if (!flSimParameters.simulationSetupUrn.empty()) {
      simulation_props.store_string("simulationSetupUrn",
                                    flSimParameters.simulationSetupUrn,
                                    "Simulation setup URN");
    } else if (!flSimParameters.simulationManagerParams.manager_config_filename
                    .empty()) {
      simulation_props.store_path(
          "simulationManagerConfig",
          flSimParameters.simulationManagerParams.manager_config_filename,
          "Simulation manager configuration file");
    }
  }

  if (flSimParameters.doDigitization) {
    // Digitization section:
    datatools::properties& digitization_props = flSimMetadata.add_section(
        "flsimulate.digitization", "flsimulate::section");
    digitization_props.set_description("Digitization setup parameters");

    // Not implemented yet.
  }

  // Variants section:
  datatools::properties& variants_props = flSimMetadata.add_section(
      "flsimulate.variantService", "flsimulate::section");
  variants_props.set_description("Variant setup");

  if (!flSimParameters.variantConfigUrn.empty()) {
    variants_props.store_string("configUrn", flSimParameters.variantConfigUrn,
                                "Variants setup configuration URN");
  } else if (!flSimParameters.variantSubsystemParams.config_filename.empty()) {
    variants_props.store_path(
        "config", flSimParameters.variantSubsystemParams.config_filename,
        "Variants setup configuration path");
  }

  if (!flSimParameters.variantProfileUrn.empty()) {
    variants_props.store_string("profileUrn", flSimParameters.variantProfileUrn,
                                "Variants profile URN");
  } else if (!flSimParameters.variantSubsystemParams.profile_load.empty()) {
    variants_props.store_path(
        "profile", flSimParameters.variantSubsystemParams.profile_load,
        "Variants profile path");
  }

  if (flSimParameters.variantSubsystemParams.settings.size()) {
    variants_props.store("settings",
                         flSimParameters.variantSubsystemParams.settings,
                         "Variants settings");
  }

  // Services section:
  datatools::properties& services_props =
      flSimMetadata.add_section("flsimulate.services", "flsimulate::section");
  services_props.set_description("Services configuration");

  if (!flSimParameters.servicesSubsystemConfigUrn.empty()) {
    services_props.store_string("configUrn",
                                flSimParameters.servicesSubsystemConfigUrn,
                                "Services setup configuration URN");
  } else if (!flSimParameters.servicesSubsystemConfig.empty()) {
    services_props.store_path("config", flSimParameters.servicesSubsystemConfig,
                              "Services setup configuration path");
  }

  return code;
}

//----------------------------------------------------------------------
falaise::exit_code do_fltranslate(int argc, char* argv[]) {
  std::string outputFileName;
  std::string inputFileName;
  int startevent, endevent;

  validation_struct validationData;

  GetOpt::GetOpt_pp ops(argc, argv);

  // Check for help request
  if (ops >> GetOpt::OptionPresent('h', "help")) {
    showHelp();
    falaise::exit_code code = falaise::EXIT_OK;
    return code;
  }

  ops >> GetOpt::Option('i', inputFileName, "");
  ops >> GetOpt::Option('s', "start", startevent, 0);  // first event default
  ops >> GetOpt::Option('e', "end", endevent, 0);      // final event default
  ops >> GetOpt::Option('o', outputFileName, "");

  if (inputFileName == "") {
    std::cout << "Need an input file name to translate, exiting." << std::endl;
    falaise::exit_code code = falaise::EXIT_UNAVAILABLE;
    return code;
  }
  if (outputFileName == "") {
    std::cout << "No output file name, using /tmp/output.brio" << std::endl;
    outputFileName = "/tmp/output.brio";
  }

  // input reader
  TFile* ff = new TFile(inputFileName.c_str());
  TTreeReader reader("hit_tree", ff);
  // obtain all the required input data from file
  TTreeReaderValue<std::vector<double>> radius(reader, "radius");
  TTreeReaderValue<std::vector<double>> wirex(reader, "wirex");
  TTreeReaderValue<std::vector<double>> wirey(reader, "wirey");
  TTreeReaderValue<std::vector<double>> wirez(reader, "wirez");
  TTreeReaderValue<std::vector<int>> grid_id(reader, "grid_id");
  TTreeReaderValue<std::vector<int>> grid_side(reader, "grid_side");
  TTreeReaderValue<std::vector<int>> grid_layer(reader, "grid_layer");
  TTreeReaderValue<std::vector<int>> grid_column(reader, "grid_column");
  TTreeReaderValue<std::vector<int>> calo_id(reader, "calo_id");
  TTreeReaderValue<std::vector<int>> calo_type(reader, "calo_type");
  TTreeReaderValue<std::vector<int>> calo_side(reader, "calo_side");
  TTreeReaderValue<std::vector<int>> calo_wall(reader, "calo_wall");
  TTreeReaderValue<std::vector<int>> calo_column(reader, "calo_column");
  TTreeReaderValue<std::vector<int>> calo_row(reader, "calo_row");

  // decide after opening ROOT file
  if (endevent <= startevent)
    endevent = reader.GetEntries(true);  // no input, read to end
  else if (endevent > reader.GetEntries(true))
    endevent = reader.GetEntries(true);  // limit maximum event number
  std::cout << "translate from event " << startevent << " to " << endevent
            << std::endl;

  // - Run:
  falaise::exit_code code = falaise::EXIT_OK;
  FLSimulate::FLSimulateArgs flSimParameters;
  // - Default Config
  flSimParameters = FLSimulate::FLSimulateArgs::makeDefault();
  try {
    // Setup services, geometry only:
    datatools::service_manager services("flTranslationServices",
                                        "SuperNEMO Translation Services");

    datatools::properties services_config;
    services_config.store("name", "flTranslationServices");
    services_config.store(
        "description",
        "SuperNEMO Demonstrator Service manager for translation");
    std::vector<std::string> scf = {
        "@falaise:config/snemo/demonstrator/geometry/4.0/"
        "geometry_service.conf"};
    services_config.store("services.configuration_files", scf);
    services.initialize(services_config);

    // Output metadata management:
    datatools::multi_properties flSimMetadata(
        "name", "type", "Metadata associated to a flsimulate run");
    do_metadata(flSimParameters, flSimMetadata);

    // Simulation output module:
    dpp::output_module simOutput;
    simOutput.set_name("FLTranslateOutput");
    simOutput.set_single_output_file(outputFileName);
    // Metadata management:
    datatools::multi_properties& metadataStore =
        simOutput.grab_metadata_store();
    metadataStore = flSimMetadata;
    simOutput.initialize_simple();

    // Manual Event loop....
    datatools::things workItem;
    dpp::base_module::process_status status;

    // set up CD data bank
    snemo::datamodel::calibrated_data* ptr_calib_data = 0;
    ptr_calib_data = &(workItem.add<snemo::datamodel::calibrated_data>("CD"));
    snemo::datamodel::calibrated_data& the_calibrated_data = *ptr_calib_data;

    unsigned int type, side, layer, column;
    unsigned int wall, row;
    double rad, wx, wy, wz;
    double rad_err = 0.9;  // default [mm]
    double z_err = 10.0;   // default [mm]
    double en = 1.0;       // dummy values
    double sen = 0.07;     // dummy values
    double time = 1.1;     // dummy values
    double st = 0.1;       // dummy values
    int tp;
    int counter = 0;
    for (int i = startevent; i < endevent; ++i) {
      // fill data vectors from root
      reader.Next();  // reader primed and pointing to data in file

      // work on tracker hits translation
      counter = 0;
      //	std::cout << "tracker hits " << grid_id->size() << " to work
      // on." << std::endl;
      for (int& id : *grid_id) {
        // create the calibrated tracker hit to build:
        snemo::datamodel::calibrated_data::tracker_hit_handle_type
            the_hit_handle(new snemo::datamodel::calibrated_tracker_hit);
        snemo::datamodel::calibrated_tracker_hit& the_calibrated_tracker_hit =
            the_hit_handle.grab();

        the_calibrated_tracker_hit.set_hit_id(id);  // from base_hit class

        type = 1204;  // geiger cell core
        side = (unsigned int)grid_side->at(counter);
        layer = (unsigned int)grid_layer->at(counter);
        column = (unsigned int)grid_column->at(counter);
        const geomtools::geom_id& gid =
            geomtools::geom_id(type, 0, side, layer, column);  // constructor
        the_calibrated_tracker_hit.set_geom_id(gid);  // from base_hit class

        rad = radius->at(counter);
        wx = wirex->at(counter);
        wy = wirey->at(counter);
        wz = wirez->at(counter);
	if (rad<rad_err) rad = rad_err;
        the_calibrated_tracker_hit.set_r(rad);
        the_calibrated_tracker_hit.set_sigma_r(rad_err);
        the_calibrated_tracker_hit.set_z(wz);
        the_calibrated_tracker_hit.set_sigma_z(z_err);
        the_calibrated_tracker_hit.set_xy(wx, wy);
	the_calibrated_tracker_hit.set_delayed(false);
        counter++;

        // save the calibrate tracker hit:
        the_calibrated_data.calibrated_tracker_hits().push_back(the_hit_handle);
        //	  std::cout << "tracker hit " << id << " pushed back." <<
        // std::endl;
      }

      // work on calorimeter hits translation
      counter = 0;
      //	std::cout << "calo hits " << calo_id->size() << " to work on."
      //<< std::endl;
      for (int& id2 : *calo_id) {
        // create a new calorimeter hit !
        snemo::datamodel::calibrated_data::calorimeter_hit_handle_type
            new_handle(new snemo::datamodel::calibrated_calorimeter_hit);
        snemo::datamodel::calibrated_calorimeter_hit&
            the_calibrated_calorimeter_hit = new_handle.grab();

        the_calibrated_calorimeter_hit.set_hit_id(id2);  // from base_hit class

        tp = calo_type->at(counter);
        side = (unsigned int)calo_side->at(counter);
        wall = (unsigned int)calo_wall->at(counter);  // xwall: 0,1; gveto: 0,1
        layer = (unsigned int)calo_column->at(
            counter);  // main calo: 0-19; gveto: 0-15; xwall: 0-1
        row =
            (unsigned int)calo_row->at(counter);  // main calo: 0-12; xwall 0-15

        if (tp == 0) {
          type = 1302;  // calorimter
          const geomtools::geom_id& gid = geomtools::geom_id(
              type, 0, side, layer, row,
              geomtools::geom_id::ANY_ADDRESS);  // constructor
          the_calibrated_calorimeter_hit.set_geom_id(
              gid);  // from base_hit class
        } else if (tp == 1) {
          type = 1232;  // x-wall
          const geomtools::geom_id& gid = geomtools::geom_id(
	     type, 0, side, wall, layer, row);  // constructor
          the_calibrated_calorimeter_hit.set_geom_id(
              gid);  // from base_hit class
        } else if (tp == 2) {
          type = 1252;  // gveto
          const geomtools::geom_id& gid = geomtools::geom_id(
              type, 0, side, wall, layer);  // constructor
          the_calibrated_calorimeter_hit.set_geom_id(
              gid);  // from base_hit class
        }

        the_calibrated_calorimeter_hit.set_time(time);
        the_calibrated_calorimeter_hit.set_sigma_time(st);
        the_calibrated_calorimeter_hit.set_energy(en);
        the_calibrated_calorimeter_hit.set_sigma_energy(sen);
        counter++;

        // save the calibrate calorimeter hit:
        the_calibrated_data.calibrated_calorimeter_hits().push_back(new_handle);
        //	  std::cout << "calo hit " << id2 << " pushed back." <<
        // std::endl;
      }

      // write workItem
      status = simOutput.process(workItem);
      if (status != dpp::base_module::PROCESS_OK) {
        std::cerr << "fltranslate : Output module failed" << std::endl;
        code = falaise::EXIT_UNAVAILABLE;
      }
      //	std::cout << "written to brio." << std::endl;

      if (code != falaise::EXIT_OK) {
        break;
      }

      // clear for next event
      the_calibrated_data.reset();
      //	std::cout << "workitem cleared." << std::endl;
    }
  } catch (std::exception& e) {
    std::cerr << "fltranslate : Setup/run of translation threw exception"
              << std::endl;
    std::cerr << e.what() << std::endl;
    code = falaise::EXIT_UNAVAILABLE;
  }

  ff->Close();
  return code;
}
}  // namespace FLTranslate
