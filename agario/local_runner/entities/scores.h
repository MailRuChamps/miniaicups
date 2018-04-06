#ifndef SCORES_H
#define SCORES_H

#include <memory>

class Scores {
public:
    inline Scores();
    inline Scores& operator +=(unsigned additionalScores);
    inline operator unsigned() const;
    /**
     * @return количество очков, полученных с момента последнего вызова extractChanges() либо с момента
     * конструирования (если extractChanges() еще не вызывался)
     */
    inline unsigned extract_changes();
private:
    std::shared_ptr<unsigned> scores;
    std::shared_ptr<unsigned> scores_delta;
};

Scores::Scores()
    :scores(std::make_shared<unsigned>(0u)),
     scores_delta(std::make_shared<unsigned>(0u)) {
}

Scores& Scores::operator+=(unsigned additionalScores) {
    *scores_delta += additionalScores;
    return *this;
}

Scores::operator unsigned() const {
    return (*scores) + (*scores_delta);
}

unsigned Scores::extract_changes() {
    unsigned delta = *scores_delta;
    *scores_delta = 0u;
    (*scores) += delta;
    return delta;
}

#endif //SCORES_H
