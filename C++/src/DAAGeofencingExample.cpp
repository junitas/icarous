/* DAA and Geofencing Example example integrating
 *   - DAIDALUS for detect and avoid.
 *   - Polycarp for geofence containment checks.
 * 
 * Copyright (c) 2011-2016 United States Government as represented by
 * the National Aeronautics and Space Administration.  No copyright
 * is claimed in the United States under Title 17, U.S.Code. All Other
 * Rights Reserved.
 */

#include <iostream>
#include <vector>
#include "Daidalus.h"
#include "SimplePoly.h"
#include "CDPolycarp.h"
#include "PolycarpResolution.h"
#include "Projection.h"
#include "Constants.h"
#include "Icarous.h"

using namespace larcfm;

void printTimeToViolation(Daidalus& daa) {
	// Aircraft at index 0 is ownship
	for (int ac_idx=1; ac_idx <= daa.lastTrafficIndex(); ++ac_idx) {
		double t2los = daa.timeToViolation(ac_idx);
		if (t2los >= 0) {
			std::cout << "Predicted time to violation with " << daa.getAircraftState(ac_idx).getId() << ": " <<
					Fm2(t2los) << " [s]" << std::endl;
		}
	}
}

void printAlerts(Daidalus& daa) {
	// Aircraft at index 0 is ownship
	for (int ac_idx=1; ac_idx <= daa.lastTrafficIndex(); ++ac_idx) {
		int alert = daa.alerting(ac_idx);
		if (alert > 0) {
			std::cout << "Alert Level " << alert << " with " <<
					daa.getAircraftState(ac_idx).getId() << std::endl;
		}
	}
}

// Converts numbers, possible NaN or infinities, to string
static std::string num2str(double res, const std::string& u) {
	if (ISNAN(res)) {
		return "N/A";
	} else if (!ISFINITE(res)) {
		return "None";
	} else {
		return Fm2(res)+" ["+u+"]";
	}
}

void printBands(Daidalus& daa, KinematicMultiBands& bands) {
	bool nowind = daa.getWindField().isZero();
	TrafficState own = daa.getOwnshipState();
	std::string trkstr = nowind ? "Track" : "Heading";
	std::string gsstr = nowind ? "Ground Speed" : "Airspeed";
	std::cout << std::endl;

	for (int alert_level = 1; alert_level <= daa.parameters.alertor.mostSevereAlertLevel(); ++alert_level) {
		std::cout << "Conflict Aircraft for Alert Level " << Fmi(alert_level) << ": " <<
				TrafficState::listToString(bands.conflictAircraft(alert_level)) << std::endl;
	}

	std::cout << std::endl;

	// Track/Heading
	double trk_deg = own.track("deg");
	std::cout << "Ownship " << trkstr << ": "+Fm2(trk_deg) << " [deg]" << std::endl;
	std::cout << "Region of Current " << trkstr+": " <<
			BandsRegion::to_string(bands.regionOfTrack(trk_deg,"deg")) << std::endl;
	std::cout << trkstr << " Bands [deg,deg]" << std::endl;
	for (int i=0; i < bands.trackLength(); ++i) {
		Interval ii = bands.track(i,"deg");
		std::cout << "  " << BandsRegion::to_string(bands.trackRegion(i)) << ":\t" << ii.toString(2) << std::endl;
	}
	std::cout << trkstr << " Resolution (right): " << num2str(bands.trackResolution(true,"deg"),"deg") << std::endl;
	std::cout << trkstr << " Resolution (left): " << num2str(bands.trackResolution(false,"deg"),"deg") << std::endl;
	std::cout << "Preferred "+trkstr+" Direction: ";
	if (bands.preferredTrackDirection()) {
		std::cout << "right" << std::endl;
	} else {
		std::cout << "left" << std::endl;
	}
	std::cout << "Time to " << trkstr << " Recovery: " << num2str(bands.timeToTrackRecovery(),"s") << std::endl;

	// Ground Speed/Air Speed
	double gs_knot = own.groundSpeed("knot");
	std::cout << "Ownship " << gsstr << ": "+Fm2(gs_knot) << " [knot]" << std::endl;
	std::cout << "Region of Current " << gsstr+": " <<
			BandsRegion::to_string(bands.regionOfGroundSpeed(gs_knot,"knot")) << std::endl;
	std::cout << gsstr << " Bands [knot,knot]:" << std::endl;
	for (int i=0; i < bands.groundSpeedLength(); ++i) {
		Interval ii = bands.groundSpeed(i,"knot");
		std::cout << "  " << BandsRegion::to_string(bands.groundSpeedRegion(i)) << ":\t" << ii.toString(2) << std::endl;
	}
	std::cout << gsstr << " Resolution (up): " << num2str(bands.groundSpeedResolution(true,"knot"),"knot") << std::endl;
	std::cout << gsstr << " Resolution (down): " << num2str(bands.groundSpeedResolution(false,"knot"),"knot") << std::endl;
	std::cout << "Preferred "+gsstr+" Direction: ";
	if (bands.preferredGroundSpeedDirection()) {
		std::cout << "up" << std::endl;
	} else {
		std::cout << "down" << std::endl;
	}
	std::cout << "Time to " << gsstr << " Recovery: " << num2str(bands.timeToGroundSpeedRecovery(),"s") << std::endl;

	// Vertical Speed
	double vs_fpm = own.verticalSpeed("fpm");
	std::cout << "Ownship Vertical Speed: "+Fm2(vs_fpm) << " [fpm]" << std::endl;
	std::cout << "Region of Current Vertical Speed: " <<
			BandsRegion::to_string(bands.regionOfVerticalSpeed(vs_fpm,"fpm")) << std::endl;
	std::cout << "Vertical Speed Bands [fpm,fpm]:" << std::endl;
	for (int i=0; i < bands.verticalSpeedLength();  ++i) {
		Interval ii = bands.verticalSpeed(i,"fpm");
		std::cout << "  " << BandsRegion::to_string(bands.verticalSpeedRegion(i)) << ":\t" << ii.toString(2) << std::endl;
	}
	std::cout << "Vertical Speed Resolution (up): " << num2str(bands.verticalSpeedResolution(true,"fpm"),"fpm") << std::endl;
	std::cout << "Vertical Speed Resolution (down): " << num2str(bands.verticalSpeedResolution(false,"fpm"),"fpm") << std::endl;
	std::cout << "Preferred Vertical Speed Direction: ";
	if (bands.preferredVerticalSpeedDirection()) {
		std::cout << "up" << std::endl;
	} else {
		std::cout << "down" << std::endl;
	}
	std::cout << "Time to Vertical Speed Recovery: " << num2str(bands.timeToVerticalSpeedRecovery(),"s") << std::endl;

	// Altitude
	double alt_ft = own.altitude("ft");
	std::cout << "Ownship Altitude: "+Fm2(alt_ft) << " [ft]" << std::endl;
	std::cout << "Region of Current Altitude: " <<
			BandsRegion::to_string(bands.regionOfAltitude(alt_ft,"ft")) << std::endl;
	std::cout << "Altitude Bands [ft,ft]:" << std::endl;
	for (int i=0; i < bands.altitudeLength(); ++i) {
		Interval ii = bands.altitude(i,"ft");
		std::cout << "  " << BandsRegion::to_string(bands.altitudeRegion(i)) << ":\t" << ii.toString(2) << std::endl;
	}
	std::cout << "Altitude Resolution (up): " << num2str(bands.altitudeResolution(true,"ft"),"ft") << std::endl;
	std::cout << "Altitude Resolution (down): " << num2str(bands.altitudeResolution(false,"ft"),"ft") << std::endl;
	std::cout << "Preferred Altitude Direction: ";
	if (bands.preferredAltitudeDirection()) {
		std::cout << "up" << std::endl;
	} else {
		std::cout << "down" << std::endl;
	}
	std::cout << "Time to Altitude Recovery: " << num2str(bands.timeToAltitudeRecovery(),"s") << std::endl;
	std::cout << std::endl;
}

int main(int argc,char* argv[]) {

	std::cout << "##" << std::endl;
	std::cout << "## "+Icarous_t::release() << std::endl;
	std::cout << "## DAA Provided by DAIDALUS" << std::endl;
	std::cout << "## Geofencing Provided by PolyCARP" << std::endl;
	std::cout << "##" << std::endl << std::endl;

	/** Detect and Avoid **/

	// Create an empty DAIDALUS object
	Daidalus daa;

	// Load parameters for a small UAS
	std::string conf_file = "params/DaidalusQuadConfig.txt";
	if (!daa.parameters.loadFromFile(conf_file)) {
		std::cout << "File " << conf_file << " not found" << std::endl;
		exit(0);
	} else {
		std::cout << "Loading DAA confiuration file " << conf_file << std::endl;
	}

	double t = 0.0;
	// for all times t (in this example, only one time step is illustrated)
	// Add ownship state at time t
	Position so = Position::makeLatLonAlt(37.1021913,"deg", -76.3869528,"deg", 5.0,"m");
	Velocity vo = Velocity::makeTrkGsVs(90.0,"deg",  0.94,"m/s", 0.0,"fpm");
	daa.setOwnshipState("ownship",so,vo,t);

	// Add all traffic states at time t
	// ... some traffic ...
	Position si = Position::makeLatLonAlt(37.10217667,"deg", -76.38679504,"deg", 5.0,"m");
	Velocity vi = Velocity::makeTrkGsVs(270.023339,"deg", 0.5,"m/s", 0.0,"fpm");
	daa.addTrafficState("ith-intruder",si,vi);
	// ... more traffic ...

	// Set wind information (TO direction)
	Velocity wind = Velocity::makeTrkGsVs(35,"deg", 1,"knot", 0,"fpm");
	daa.setWindField(wind);

	// Print information about the Daidalus Object
	std::cout << "Number of Aircraft: " << daa.numberOfAircraft() << std::endl;
	std::cout << "Last Aircraft Index: " << daa.lastTrafficIndex() << std::endl;
	std::cout <<  std::endl;

	// Check time to violation
	printTimeToViolation(daa);

	// Call alerting logic for each traffic aircraft.
	printAlerts(daa);

	// Compute resolution bands
	KinematicMultiBands bands;
	daa.kinematicMultiBands(bands);

	// Print track, ground speed, vertical speed and altitude bands
	printBands(daa,bands);
	// continue with next time step

	/** Geofencing **/

	// Add geofence
	double floor     = 0;  // 0 m
	double ceiling   = 10; // 10 m

	// Make a geofence with 4 vertices (keep in fence)
	SimplePoly geoPolyLLA1(floor,ceiling);
	geoPolyLLA1.addVertex(Position::makeLatLonAlt(37.102545,"deg",-76.387213,"deg",0,"m"));
	geoPolyLLA1.addVertex(Position::makeLatLonAlt(37.102344,"deg",-76.387163,"deg",0,"m"));
	geoPolyLLA1.addVertex(Position::makeLatLonAlt(37.102351,"deg",-76.386844,"deg",0,"m"));
	geoPolyLLA1.addVertex(Position::makeLatLonAlt(37.102575,"deg",-76.386962,"deg",0,"m"));

	CDPolycarp geoPolyCarp;
	PolycarpResolution geoRes;

	// Project geofence vertices to a local euclidean coordinate system to use with polycarp functions
	EuclideanProjection proj  = Projection::createProjection(geoPolyLLA1.getVertex(0));
	Poly3D geoPoly3D1          = geoPolyLLA1.poly3D(proj);

	Vect3 so_3 = proj.project(so); // Project ownship position into local euclidean frame
	if (geoPolyCarp.definitelyInside(so_3,geoPoly3D1)) {
		std::cout << "Definitely inside of keep in fence\n";
	}

	if (geoPolyCarp.definitelyOutside(so_3,geoPoly3D1)) {
		std::cout << "Definitely outside of keep in fence\n";
	}

	std::vector<Vect2> fenceVertices;
	fenceVertices.reserve(4);

	for(int i=0;i<4;i++) {
		fenceVertices.push_back(geoPoly3D1.getVertex(i));
	}

	//Check if ownship is near any edge (nearness is defined based on horizontal and vertical thresholds)
	double hthreshold = 1; // 1 m
	double vthreshold = 1; // 1 m
	if (geoPolyCarp.nearEdge(so_3,geoPoly3D1,hthreshold,vthreshold)) {
		std::cout << "Ownship is near geofence edge";

		// Compute a safe point to goto
		double BUFF = 0.1; // Buffer used for numerical stability within polycarp
		Vect2 so_2  = so_3.vect2(); // 2D projection of ownship position

		// Compute safe point
		Vect2 recpoint = geoRes.inside_recovery_point(BUFF,hthreshold,fenceVertices,so_2);

		// Convert safe point from euclidean coordinates to Lat, Lon and Alt
		LatLonAlt LLA = proj.inverse(recpoint,so.alt());
		Position RecoveryPoint = Position::makeLatLonAlt(LLA.latitude(),LLA.longitude(),LLA.altitude());
		std::cout << RecoveryPoint.toString();
	}

}
