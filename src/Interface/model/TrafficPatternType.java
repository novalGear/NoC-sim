package Interface.model;

public enum TrafficPatternType {
    UNIFORM("UNIFORM"),
    HOTSPOT("HOTSPOT"),
    SYNTHETIC("SYNTHETIC");

    private final String value;

    TrafficPatternType(String value) {
        this.value = value;
    }

    public String getValue() {
        return value;
    }

    @Override
    public String toString() {
        return value;
    }
}
