package core.hydracore.repository;

import core.hydracore.entity.WaterLevel;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

import java.util.List;

@Repository
public interface WaterLevelRepository extends JpaRepository<WaterLevel, Long> {
    @Override
    List<WaterLevel> findAll();

}
