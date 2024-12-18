# Stage 1: build
# Start with a maven image that includes JDK 21 to build the application
FROM maven:3.9.8-amazoncorretto-21 AS build

# Copy the source code and pom.xml to /app folder
WORKDIR /app
COPY pom.xml .
COPY src ./src

# Build source code with maven
RUN mvn package -DskipTests

# Stage 2: create a image
# Start with Amazon Corretto JDK 21
FROM amazoncorretto:21.0.4

# Set working folder to App and copy complied file from above step
WORKDIR /app
COPY --from=build /app/target/*.jar app.jar

# Command to run the application
ENTRYPOINT ["java", "-jar", "app.jar"]