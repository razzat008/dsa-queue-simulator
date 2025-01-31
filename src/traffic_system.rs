use std::cmp::Ordering;
use std::collections::BinaryHeap;

#[derive(Eq, PartialEq, Debug)]
pub struct Lane {
    pub id: String,
    pub vechile_count: usize,
    pub priority: usize,
}

impl Ord for Lane {
    fn cmp(&self, other: &self) -> Ordering {
        other.priority.cmp(&self.priority)
    }
}

impl PartialOrd for Lane {
    fn partial_cmp(&self, other: &self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}
