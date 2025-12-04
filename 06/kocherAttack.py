import re
from dataclasses import dataclass
from typing import List

N = 10961
E = 5
INPUT_FILE = "100times.txt"


@dataclass
class Sample:
    x: int
    sig: int
    time: int


def parse_samples(path: str) -> List[Sample]:
    samples: List[Sample] = []
    with open(path, "r") as f:
        for line in f:
            line = line.strip()
            if (not line
                    or line.startswith("//")
                    or line.startswith("#")
                    or line.startswith("x ")):
                continue
            m = re.match(r"(\d+)\s*\((\d+),\s*(\d+)\)", line)
            if not m:
                continue
            x = int(m.group(1))
            sig = int(m.group(2))
            t = int(m.group(3))
            samples.append(Sample(x, sig, t))
    return samples

def simulate_get_s_before_bit(M: int, bits: List[int]) -> int:
    """
    Simulate the algorithm with given bits and return s value
    AFTER processing all bits (which is s BEFORE the next bit).
    """
    s = 1
    for b in bits:
        if b == 1:
            r = (s * M) % N
        else:
            r = s
        s = (r * r) % N
    return s


def compute_correlation(samples: List[Sample], bits: List[int]) -> float:
    """Compute timing correlation for hypothesis that next bit is 1."""
    slow_times = []
    fast_times = []
    
    for sample in samples:
        M = sample.x
        s = simulate_get_s_before_bit(M, bits)
        
        if s % 100 == 0:
            slow_times.append(sample.time)
        else:
            fast_times.append(sample.time)
    
    if len(slow_times) < 5 or len(fast_times) < 5:
        return 0.0
    
    avg_slow = sum(slow_times) / len(slow_times)
    avg_fast = sum(fast_times) / len(fast_times)
    
    return avg_slow - avg_fast


def compute_total_correlation(samples: List[Sample], bits: List[int]) -> float:
    """
    Compute the sum of correlations for all bit positions where bit=1.
    Higher value means better match with timing data.
    """
    total = 0.0
    for i in range(len(bits)):
        if bits[i] == 1:
            # Correlation at position i (using bits before position i)
            corr = compute_correlation(samples, bits[:i])
            total += corr
    return total


def kocher_attack_lookahead(samples: List[Sample], bit_length: int, depth: int = 4) -> List[int]:
    """
    Kocher timing attack using lookahead for every bit decision.
    For each bit, try both 0 and 1, then look ahead 'depth' bits to see
    which choice leads to better overall correlation.
    """
    recovered_bits = []
    
    for bit_pos in range(bit_length):
        best_bit = 0
        best_score = float('-inf')
        
        for test_bit in [0, 1]:
            test_bits = recovered_bits + [test_bit]
            
            # Look ahead: try all combinations for the next 'depth' bits
            remaining = min(depth, bit_length - bit_pos - 1)
            score = find_best_correlation(samples, test_bits, remaining)
            
            if score > best_score:
                best_score = score
                best_bit = test_bit
        
        recovered_bits.append(best_bit)
    
    return recovered_bits


def find_best_correlation(samples: List[Sample], bits: List[int], depth: int) -> float:
    """
    Recursively find the best total correlation by trying all combinations
    for the next 'depth' bits.
    """
    if depth == 0:
        return compute_total_correlation(samples, bits)
    
    # Try both 0 and 1 for the next bit
    score_0 = find_best_correlation(samples, bits + [0], depth - 1)
    score_1 = find_best_correlation(samples, bits + [1], depth - 1)
    
    return max(score_0, score_1)


def bits_to_int(bits: List[int]) -> int:
    """Convert MSB-first bit list to integer."""
    result = 0
    for b in bits:
        result = (result << 1) | b
    return result


def main():
    samples = parse_samples(INPUT_FILE)
    if not samples:
        print("No samples found, check 100times.txt")
        return

    print(f"Loaded {len(samples)} samples from {INPUT_FILE}")    
    
    bit_length = 13
    
    recovered_bits = kocher_attack_lookahead(samples, bit_length, depth=4)
    print(f"Recovered d (binary): {''.join(map(str, recovered_bits))}")


if __name__ == "__main__":
    main()