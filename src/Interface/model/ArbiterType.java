package Interface.model;

public enum ArbiterType {
    RR("RR"),
    PRIORITY("PRIORITY"),
    LRU("LRU");

    private final String value;

    ArbiterType(String value) {
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
