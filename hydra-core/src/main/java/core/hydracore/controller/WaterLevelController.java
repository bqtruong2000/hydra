package core.hydracore.controller;

import core.hydracore.entity.WaterLevel;
import core.hydracore.service.WaterLevelService;
import lombok.RequiredArgsConstructor;
import lombok.experimental.FieldDefaults;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.util.List;

@Controller
@RestController
@RequestMapping("/mqtt/water-level")
@RequiredArgsConstructor
@FieldDefaults(level = lombok.AccessLevel.PRIVATE, makeFinal = true)
public class WaterLevelController {
    @Autowired
    WaterLevelService waterLevelService;

    @GetMapping("/all")
    List<WaterLevel> getAll() {
        return waterLevelService.getAllWaterLevels();
    }
}
