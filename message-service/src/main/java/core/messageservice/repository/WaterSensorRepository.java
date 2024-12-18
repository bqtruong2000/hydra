package core.messageservice.repository;

import core.messageservice.entity.WaterLevel;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

@Repository
public interface WaterSensorRepository extends JpaRepository<WaterLevel, Long> {
}
