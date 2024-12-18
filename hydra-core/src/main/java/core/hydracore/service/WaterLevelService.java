package core.hydracore.service;

import core.hydracore.entity.WaterLevel;
import core.hydracore.repository.WaterLevelRepository;
import lombok.RequiredArgsConstructor;
import lombok.experimental.FieldDefaults;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.Comparator;
import java.util.List;

@Service
@RequiredArgsConstructor
@FieldDefaults(level = lombok.AccessLevel.PRIVATE, makeFinal = true)
@Slf4j
public class WaterLevelService {
    @Autowired
    WaterLevelRepository waterLevelRepository;


    public List<WaterLevel> getAllWaterLevels() {
        return waterLevelRepository.findAll();
    }
}
