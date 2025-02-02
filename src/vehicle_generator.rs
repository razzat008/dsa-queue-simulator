use rand::Rng;
use crate::traffic_system::TrafficSystem;

pub fn generate_random_vehicles(traffic_system: &mut TrafficSystem) {
    let lanes = ["AL2", "BL1", "CL1", "DL1"];
    let mut rng = rand::thread_rng();

    for lane in lanes.iter() {
        let new_vehicles = rng.gen_range(1..5);
        traffic_system.add_lane(lane, new_vehicles);
    }
}

