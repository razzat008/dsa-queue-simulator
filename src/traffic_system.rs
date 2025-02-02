use std::cmp::Ordering;
use std::collections::{BinaryHeap, HashMap};
// mod vechile_generator

#[derive(Eq, PartialEq, Debug, Clone)]
pub struct Lane {
    pub id: String,
    pub vehicle_count: usize,
    pub priority: usize,
}

impl Ord for Lane {
    fn cmp(&self, other: &Self) -> Ordering {
        self.priority.cmp(&other.priority) // Max-Heap: Higher priority first
    }
}

impl PartialOrd for Lane {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

pub struct TrafficSystem {
    lanes: HashMap<String, Lane>, // Store lanes for quick access
    queue: BinaryHeap<Lane>,      // Priority queue for ordering
}

impl TrafficSystem {
    // initialize the TrafficSystem
    pub fn new() -> Self {
        Self {
            lanes: HashMap::new(),
            queue: BinaryHeap::new(),
        }
    }

    pub fn add_lane(&mut self, id: &str, vehicle_count: usize) {
        let priority = Self::calculate_priority(id, vehicle_count);
        let lane = Lane {
            id: id.to_string(),
            vehicle_count,
            priority,
        };
        self.lanes.insert(id.to_string(), lane.clone());
        self.queue.push(lane);
    }

    fn calculate_priority(id: &str, vehicle_count: usize) -> usize {
        if id == "AL2" && vehicle_count > 10 {
            return 1000;
        }
        vehicle_count
    }

    pub fn serve_vehicle(&mut self) {
        if let Some(lane) = self.queue.pop() {
            if let Some(lane_entry) = self.lanes.get_mut(&lane.id) {
                println!(
                    "🚦 Serving lane: {} | Vehicles before: {}",
                    lane.id, lane_entry.vehicle_count
                );

                if lane_entry.vehicle_count > 0 {
                    lane_entry.vehicle_count -= 1;
                    lane_entry.priority =
                        Self::calculate_priority(&lane.id, lane_entry.vehicle_count);
                }

                println!(
                    "🚦 Updated lane: {} | Vehicles after: {} | New Priority: {}",
                    lane.id, lane_entry.vehicle_count, lane_entry.priority
                );
                if lane.vehicle_count > 0 {
                    self.queue.push(lane_entry.clone());
                }
            }
        }
    }

    pub fn debug_print(&self) {
        let lanes: Vec<_> = self.queue.clone().into_sorted_vec();
        // lanes.reverse(); // Show highest priority first
        println!("📌 Current Lane Priorities:");
        for lane in &lanes {
            println!(
                "Lane: {} | Vehicles: {} | Priority: {}",
                lane.id, lane.vehicle_count, lane.priority
            );
        }
        println!("-----------------------------------");
    }
}
