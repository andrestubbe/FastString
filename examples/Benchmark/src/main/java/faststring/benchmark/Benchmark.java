package faststring.benchmark;

import faststring.FastString;
import org.openjdk.jmh.annotations.BenchmarkMode;
import org.openjdk.jmh.annotations.Fork;
import org.openjdk.jmh.annotations.Measurement;
import org.openjdk.jmh.annotations.Mode;
import org.openjdk.jmh.annotations.OutputTimeUnit;
import org.openjdk.jmh.annotations.Scope;
import org.openjdk.jmh.annotations.Setup;
import org.openjdk.jmh.annotations.State;
import org.openjdk.jmh.annotations.Warmup;

import java.util.concurrent.TimeUnit;

@BenchmarkMode(Mode.Throughput)
@OutputTimeUnit(TimeUnit.SECONDS)
@State(Scope.Thread)
@Fork(1)
@Warmup(iterations = 2, time = 1)
@Measurement(iterations = 3, time = 1)
public class Benchmark {

    private String text;
    private FastString fastString;

    @Setup
    public void setup() {
        text = "FastJava Agentic Ecosystem - High-Performance SIMD String Search and Manipulation Engine for Java.";
        fastString = new FastString(text);
    }

    @org.openjdk.jmh.annotations.Benchmark
    public int testFastStringIndexOf() {
        return fastString.indexOf("SIMD");
    }

    @org.openjdk.jmh.annotations.Benchmark
    public int testJavaStringIndexOf() {
        return text.indexOf("SIMD");
    }
}
