#pragma once

#include "macro_definition.hpp"

#include <glm/glm.hpp>
#include <optional>
#include <array>
#include <algorithm>
#include <vector>
#include <complex>

namespace billiard::physics {

    const float gravitationalAcceleration = 9.8 * 1000; // mm/s^2
    const float frictionCoefficient = 0.0142435;
    const float slideFrictionCoefficient = 12.0f * frictionCoefficient; // TODO: find a good number!
    const float energyLossByBall = 0.05; // TODO: Document and find a number!
    const float energyLossFactorBall = 1 - energyLossByBall;
    const float energyAdditionFactorBall = 1 / energyLossFactorBall;
    const float topspinConstant = 100.0f; // TODO: Find a number
    const float topspinRest = 0.25f; // TODO: Find a number
    const bool isTopspinConstant = false;
    const float energyLossByRail = 0.5f; // https://www.researchgate.net/publication/245388279_A_theoretical_analysis_of_billiard_ball_dynamics_under_cushion_impacts
    const float energyLossFactorRail = 1 - energyLossByRail;
    const float energyAdditionFactorRail = 1 / energyLossFactorRail;

    /**
     * Berechnet die quadrierte Distanz eines Punktes zu einem Liniensegment.
     * @param linePoint1 Startpunkt des Liniensegments.
     * @param linePoint2 Endpunkt des Liniensegments.
     * @param point Der Punkt, zu welchem der Abstand gemessen wird
     * @return Die quadrierte Distanz zwischen dem Liniensegment und dem Punkt.
     */
    EXPORT_BILLIARD_PHYSICS_LIB float pointToLineSegmentSquaredDistance(const glm::vec2& linePoint1, const glm::vec2& linePoint2, const glm::vec2& point);

    /**
     * Berechnet die quadrierte Distanz eines Punktes zu einer Half-Line.
     * @param linePoint1 Startpunkt der Half-Line.
     * @param lineDirection Richtungsvektor der Half-Line.
     * @param point Der Punkt, zu welchem der Abstand gemessen wird
     * @return Die quadrierte Distanz zwischen der Half-Line und dem Punkt.
     */
    EXPORT_BILLIARD_PHYSICS_LIB float pointToHalfLineSquaredDistance(const glm::vec2& linePoint1, const glm::vec2& lineDirection, const glm::vec2& point);

    /**
     * Kugel A soll über eine gewisse Distanz rollen und am Ziel eine gewünschte Zielgeschwindigkeit haben. Reibung wird berücksichtigt.
     * @param targetVelocity Geschwindigkeit, welche Kugel A nach zurücklegen der Distanz innehaben soll.
     * @param distance Distanz, welche von Kugel A zurückgelegt werden muss.
     * @return Notwendige Startgeschwindigkeit von Kugel A, um nach zurücklegen der Distanz eine gewünschte Zielgeschwindigkeit zu haben.
     */
    EXPORT_BILLIARD_PHYSICS_LIB glm::vec2 startVelocity(const glm::vec2& targetVelocity, float distance);

    /**
     * Kugel A soll mit Kugel B kollidieren, Kugel B soll nach der Kollision in Richtung targetDirection rollen.
     * @param targetBallPosition Position der Kugel B.
     * @param targetDirection Richtung in die Kugel B nach der Kollision rollen soll.
     * @param ballRadius Radius der beiden Kugeln.
     * @return Zielposition der Kugel A, wohin diese gestossen werden muss, um Kugel B so zu treffen, dass diese in die gewünschte Richtung rollt.
     */
    EXPORT_BILLIARD_PHYSICS_LIB glm::vec2 elasticCollisionTargetPosition(const glm::vec2& targetBallPosition, const glm::vec2& targetDirection, float ballRadius);

    /**
     * Kugel A kollidiert mit Kugel B, Kugel B soll mit targetVelocity von der Kollision ausgehen.
     * Berechnet die notwendige Geschwindigkeit der Kugel A zum Zeitpunkt der Kollision, damit Kugel B die gewünschte Zielgeschwindigkeit hat.
     * @param targetVelocity Zielgeschwindigkeit der Kugel B nach der Kollision.
     * @param originVelocity Richtung der Kugel A, mit der sie auf Kugel B zurollt.
     * @return Geschwindigkeit, die Kugel A zum Zeitpunkt der Kollision haben muss, um Kugel B mit der gewünschten Geschwindigkeit weiterzustossen.
     */
    EXPORT_BILLIARD_PHYSICS_LIB glm::vec2 elasticCollisionReverse(const glm::vec2& targetVelocity, const glm::vec2& originVelocity);

    /**
     * Kugel A kollidiert mit Kugel B. Die Geschwindigkeitsvektoren beider Kugeln nach dem elastischen Stoss werden berechnet.
     * @param position1 Position der Kugel A zum Zeitpunkt der Kollision.
     * @param velocity1 Geschwindigkeit der Kugel A zum Zeitpunkt der Kollision.
     * @param position2 Position der Kugel B zum Zeitpunkt der Kollision.
     * @param velocity2 Geschwindigkeit der Kugel B zum Zeitpunkt der Kollision.
     * @param isRolling1 Gibt an, ob die Kugel A am rollen ist.
     * @param isRolling2 Gibt an, ob die Kugel B am rollen ist.
     * @return Die Geschwindigkeitsvektoren beider Kugeln nach dem elastischen Stoss.
     */
    EXPORT_BILLIARD_PHYSICS_LIB std::pair<glm::vec2, glm::vec2> elasticCollision(const glm::vec2& position1,
                                                                                 const glm::vec2& velocity1,
                                                                                 const glm::vec2& position2,
                                                                                 const glm::vec2& velocity2,
                                                                                 bool isRolling1,
                                                                                 bool isRolling2);

    /**
     * Kugel kollidiert mit Bande und wird dort reflektiert
     * @param velocityNormalized
     * @param normal Bandennormale
     * @return Reflektierter Geschwindigkeitsvektor
     */
    EXPORT_BILLIARD_PHYSICS_LIB glm::vec2 railCollision(const glm::vec2& velocity, const glm::vec2& normal);

    /**
     * Berechnet das Perp-Produkt.
     * @param vector Der Vektor
     * @return Das Perp-Produkt.
     */
    EXPORT_BILLIARD_PHYSICS_LIB glm::vec2 perp(glm::vec2 vector);

    /**
     * Berechnet die Dauer bis eine Kugel mit einer Bande kollidiert.
     * @param ballPosition Startposition der Kugel.
     * @param ballVelocity Startgeschwindigkeit der Kugel
     * @param acceleration Konstante Beschleunigung der Kugel
     * @param railPoint1 Startpunkt des Bandensegments für die Kollisionserkennung.
     * @param railPoint2 Endpunkt des Bandensegments für die Kollisionserkennung.
     * @param shiftedRailPoint1 Um den Kugelradius verschobener Startpunkt des Bandensegments für die Bestimmung des Kollisionszeitpunktes.
     * @param shiftedRailPoint2 Um den Kugelradius verschobener Endpunkt des  für die Bestimmung des Kollisionszeitpunktes.
     * @return Die Dauer, nach der die Kugel mit der Bande kollidiert, oder kein Resultat, wenn keine Kollision möglich ist.
     */
    EXPORT_BILLIARD_PHYSICS_LIB std::optional<float> timeToCollisionWithRail(
            const glm::vec2& ballPosition,
            const glm::vec2& ballVelocity,
            const glm::vec2& acceleration,
            const glm::vec2& railPoint1,
            const glm::vec2& railPoint2,
            const glm::vec2& shiftedRailPoint1,
            const glm::vec2& shiftedRailPoint2);

     /** Berechnet den Betrag der Bremsbeschleunigung beim Rollen.
     * @return Den Betrag der Bremsbeschleunigung
     */
    EXPORT_BILLIARD_PHYSICS_LIB float accelerationLength();

    /** Berechnet den Betrag der Bremsbeschleunigung beim Gleiten.
     * @return Den Betrag der Bremsbeschleunigung
     */
    EXPORT_BILLIARD_PHYSICS_LIB float slideAccelerationLength();

    /**
     * Gibt für eine bekannte Geschwindigkeit und Bremsbeschleunigungsbetrag die Bremsbeschleunigung in
     * entgegengesetzter Richtung an.
     * @param accelerationLength Betrag der Bremsbeschleunigung
     * @aram velocity Geschwindigkeit des Objekts
     * @return Die Bremsbeschleunigung
     */
    EXPORT_BILLIARD_PHYSICS_LIB glm::vec2 acceleration(const glm::vec2& velocity, float accelerationLength);

    /**
     * Gibt für eine bekannte Geschwindigkeit und Bremsbeschleunigungsbetrag die Bremsbeschleunigung in
     * entgegengesetzter Richtung an.
     * @param accelerationLength Betrag der Bremsbeschleunigung
     * @aram velocity Normierte Geschwindigkeit des Objekts
     * @return Die Bremsbeschleunigung
     */
    EXPORT_BILLIARD_PHYSICS_LIB glm::vec2 accelerationByNormed(const glm::vec2& velocity, float accelerationLength);

    /**
     * Wendet eine konstante Beschleunigung über eine bestimmte Zeitdauer auf einen Geschwindigkeitsvektor an.
     * @param acceleration Konstante Beschleunigung, die über eine gewisse Dauer angewendet werden soll.
     * @param currentVelocity Geschwindigkeitsvektor, auf den die Beschleunigung wirkt.
     * @param deltaTime Zeitdauer, während dieser die Beschleunigung auf den Geschwindigkeitsvektor wirkt.
     * @return Der neue Geschwindigkeitsvektor, nachdem eine konstante Beschleunigung über eine bestimmte Zeitdauer angewendet wurde.
     */
    EXPORT_BILLIARD_PHYSICS_LIB glm::vec2 accelerate(const glm::vec2& acceleration, const glm::vec2& currentVelocity, float deltaTime);

    /**
    * Berechnet die Position nach einer bestimmten Zeit, Geschwindigkeit und Beschleunigung.
    * @param acceleration Konstante Beschleunigung, die über eine gewisse Dauer angewendet werden soll.
    * @param velocity Geschwindigkeitsvektor, auf den die Beschleunigung wirkt.
    * @param deltaTime Zeitdauer, während dieser die Beschleunigung auf den Geschwindigkeitsvektor wirkt.
     *@param position Die aktuelle Position.
    * @return Die neue Position.
    */
    EXPORT_BILLIARD_PHYSICS_LIB glm::vec2 position(const glm::vec2& acceleration, const glm::vec2& velocity, float deltaTime, const glm::vec2& position);

    /**
     * Berechnet den Zeitpunkt, wo ein Objekt stillsteht.
     * @param acceleration Die konstante Beschleunigung des Objekts.
     * @param velocity Die Geschwindigkeit des Objekts.
     * @return Die Zeit, an der das Objekt stillsteht.
     */
    EXPORT_BILLIARD_PHYSICS_LIB float timeToStop(const glm::vec2& acceleration, const glm::vec2& velocity);

    /**
     * Berechnet den Zeitpunkt, wo ein Objekt zu rollen beginnt.
     * @param velocity Die Geschwindigkeit des Objekts.
     * @return Die Zeit, an der das Objekt zu rollen beginnt.
     */
    EXPORT_BILLIARD_PHYSICS_LIB float timeToRolling(const glm::vec2& velocity);

    /**
     * Berechnet die Dauer, bis zwei Kugel miteinander kollidieren.
     * @param acceleration1 Konstante Beschleunigung der Kugel A.
     * @param velocity1 Startgeschwindigkeit der Kugel A.
     * @param position1 Startposition der Kugel A.
     * @param acceleration2 Konstante Beschleunigung der Kugel B.
     * @param velocity2 Startgeschwindigkeit der Kugel B.
     * @param position2 Startposition der Kugel B.
     * @param diameter Kugeldurchmesser.
     * @return Dauer bis die beiden Kugeln kollidieren, oder kein Resultat, wenn keine Kollision stattfinden kann.
     */
    EXPORT_BILLIARD_PHYSICS_LIB std::optional<float> timeToCollision(const glm::vec2& acceleration1, const glm::vec2& velocity1,
                                                const glm::vec2& position1, const glm::vec2& acceleration2,
                                                const glm::vec2& velocity2, const glm::vec2& position2, float diameter);

    /**
     * Berechnet die Dauer, bis eine Kugel in ein Loch rollt.
     * @param targetPosition Zielposition der Kugel im Loch.
     * @param position Startposition der Kugel.
     * @param normalizedVelocity Normalisierte Startgeschwindigkeit der Kugel.
     * @param velocity Startgeschwindigkeit der Kugel.
     * @param acceleration Konstante Beschleunigung der Kugel.
     * @param radius Der Radius des Ziels.
     * @return Die Dauer, bis eine Kugel in ein Loch rollt oder kein Resultat, wenn sie nie ins Loch rollen kann.
     */
    EXPORT_BILLIARD_PHYSICS_LIB std::optional<float> timeToTarget(const glm::vec2& targetPosition,
                                                                  const glm::vec2& position,
                                                                  const glm::vec2& normalizedVelocity,
                                                                  const glm::vec2& velocity,
                                                                  const glm::vec2& acceleration,
                                                                  float radius);

    EXPORT_BILLIARD_PHYSICS_LIB std::vector<float> nonNegative(const std::vector<float>& values);

    /**
     * Berechnet einen Vektor der Länge 1, welcher in dieselbe Richtung wie vector zeigt.
     * Wenn vector 0 ist, dann wird der 0-Vektor zurückgegeben.
     * @param Der zu normalisierende Vektor.
     * @return Der normalisierte Vektor.
     */
    EXPORT_BILLIARD_PHYSICS_LIB glm::vec2 normalize(const glm::vec2& vector);

    namespace intersection {

        using LineIntersection = std::vector<float>;
        using RayIntersection = std::vector<float>;
        using LineLineIntersection = std::optional<std::pair<float, float>>;

        /**
         * Source: https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection
         */
        EXPORT_BILLIARD_PHYSICS_LIB std::vector<float> solveQuadraticFormula(float a, float b, float c);

        /**
         * See: https://de.wikipedia.org/wiki/Quartische_Gleichung#L%C3%B6sungsformel_und_Beweis
         * Löse eine Gleichung vierten Grades: ax^4 + bx^3 + cx^2 + dx + e
         */
        EXPORT_BILLIARD_PHYSICS_LIB std::vector<std::complex<double>> solveQuarticFormula(double a, double b, double c, double d, double e);

        /**
         * Test whether or not and if yes, where, a line (infinite length) intersects a circle.
         * @param center center point of circle
         * @param radius radius of circle
         * @param pointOnLine point on line
         * @param lineDirection normalized (!) direction vector of line
         * @return
         */
        EXPORT_BILLIARD_PHYSICS_LIB LineIntersection lineIntersectsCircle(const glm::vec2& center,
                                                                       float radius,
                                                                       const glm::vec2& pointOnLine,
                                                                       const glm::vec2& lineDirection);

        EXPORT_BILLIARD_PHYSICS_LIB RayIntersection rayIntersectsCircle(const glm::vec2& center,
                                                                     float radius,
                                                                     const glm::vec2& rayOrigin,
                                                                     const glm::vec2& rayDirection);

        /**
         * Test whether or not and if yes, where, a line 1 (infinite length) intersects another line 2 (infinite length).
         * @param linePoint1        a point on the first line
         * @param lineDirection1    the direction of the first line
         * @param linePoint2        a point on the second line
         * @param lineDirection2    the direction of the second line
         * @return
         */
        EXPORT_BILLIARD_PHYSICS_LIB LineLineIntersection lineIntersectsLine(const glm::vec2& linePoint1,
                                                                         const glm::vec2& lineDirection1,
                                                                         const glm::vec2& linePoint2,
                                                                         const glm::vec2& lineDirection2);

        /**
         * Test whether or not and if yes, where, a half-line (infinite length in one direction) intersects a line segment (finite length).
         * @param halfLinePoint         start point of the half-line
         * @param halfLineDirection     direction of the half-line
         * @param lineSegmentStartPoint start point of the line segment
         * @param lineSegmentEndPoint   end point of the line segment
         * @return
         */
        EXPORT_BILLIARD_PHYSICS_LIB LineLineIntersection
        halfLineIntersectsLineSegment(const glm::vec2& halfLinePoint,
                                      const glm::vec2& halfLineDirection,
                                      const glm::vec2& lineSegmentStartPoint,
                                      const glm::vec2& lineSegmentEndPoint);

        /**
         * Test whether or not and if yes, where, two half-lines (infinite length in one direction) intersect.
         * @param halfLinePoint1       start point of the first half-line
         * @param halfLineDirection1   direction of the first half-line
         * @param halfLinePoint2       start point of the second half-line
         * @param halfLineDirection2   direction of the second half-line
         * @return
         */
        EXPORT_BILLIARD_PHYSICS_LIB LineLineIntersection
        halfLineIntersectsHalfLine(const glm::vec2& halfLinePoint1,
                                      const glm::vec2& halfLineDirection1,
                                      const glm::vec2& halfLinePoint2,
                                      const glm::vec2& halfLineDirection2);

    }

}