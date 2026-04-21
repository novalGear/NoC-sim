package Interface.service;

import Interface.exception.SimulationException;
import Interface.model.SimulationConfig;
import Interface.model.SimulationResult;

public class SimulationService {
    private final NativeInterface nativeInterface;
    private final JsonConverter jsonConverter;

    public SimulationService() {
        this.nativeInterface = NativeInterface.getInstance();
        this.jsonConverter = new JsonConverter();
    }

    public SimulationResult runSimulation(SimulationConfig config) throws SimulationException {
        // Валидация конфигурации
        if (!config.isValid()) {
            throw new SimulationException("Invalid configuration: " + config.getValidationError());
        }

        try {
            // Конвертация в JSON
            String jsonConfig = jsonConverter.configToJson(config);

            // Инициализация в C++
            nativeInterface.initialize(jsonConfig);

            // Запуск симуляции
            String jsonResult = nativeInterface.runSimulation();

            // Парсинг результата
            return jsonConverter.jsonToResult(jsonResult);

        } catch (SimulationException e) {
            throw e;
        } catch (Exception e) {
            throw new SimulationException("Simulation failed: " + e.getMessage());
        } finally {
            nativeInterface.reset();
        }
    }
}
