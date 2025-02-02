#![allow(dead_code, unused_variables)]
#[derive(Debug)]
pub enum TrafficLight {
    Green,
    Red,
}

impl TrafficLight {
    fn toggle_state(&self) -> TrafficLight {
        // To toggle the states of traffic lights
        match self {
            TrafficLight::Green => TrafficLight::Red,
            TrafficLight::Red => TrafficLight::Green,
        }
    }
}

