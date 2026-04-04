---
name: commit-message
description: Analyze staged git changes and generate a commit message following Conventional Commits format
disable-model-invocation: true
argument-hint: "[type] [description]"
---

# Conventional Commit Message Generator

Staged된 git 변경 사항을 분석하여 Conventional Commits 형식의 커밋 메시지를 생성한다.

## Instructions

1. 아래 명령어를 실행하여 변경 사항을 파악한다:
   - `git diff --cached --stat` — 변경된 파일 목록과 통계
   - `git diff --cached` — 상세 변경 내용
   - `git log --oneline -10` — 최근 커밋 스타일 참고

2. staged된 변경이 없으면 사용자에게 알리고 종료한다.

3. 변경 내용을 분석하여 아래 항목을 결정한다:
   - **Type**: `feat`, `fix`, `docs`, `refactor`, `chore`, `perf`, `test`, `ci`, `build`, `style` 중 가장 적합한 것
   - **Scope** (선택): 변경된 파일/모듈에서 추론 (예: `renderer`, `input`, `build`)
   - **Subject**: 변경 사항을 요약하는 한 줄 설명
   - **Body** (선택): 변경 이유나 상세 설명이 필요한 경우에만 추가

4. `$ARGUMENTS`가 제공된 경우 해당 값을 type이나 description 힌트로 활용한다.

## Commit Message Format

```
<type>(<scope>): <subject>

[optional body]
```

## Rules

- Subject line은 50자 이내
- 명령형(imperative mood) 사용: "add feature" (O), "added feature" (X)
- Subject 첫 글자는 소문자
- Subject 끝에 마침표 없음
- Body는 72자에서 줄바꿈
- Body에서 "무엇을"이 아닌 "왜"를 설명

## Output

- 최종 커밋 메시지를 코드 블록으로 출력한다
- **git commit을 실행하지 않는다** — 메시지 제안만 수행
- 메시지가 여러 개 가능한 경우 가장 적합한 하나만 제안

## Type Reference

| Type       | Description                              |
|------------|------------------------------------------|
| `feat`     | 새로운 기능 추가                          |
| `fix`      | 버그 수정                                |
| `docs`     | 문서 변경                                |
| `style`    | 코드 스타일 (포매팅, 세미콜론 등)          |
| `refactor` | 동작 변경 없는 코드 리팩토링               |
| `perf`     | 성능 개선                                |
| `test`     | 테스트 추가/수정                          |
| `chore`    | 빌드, 의존성, 도구 설정                    |
| `ci`       | CI/CD 설정 변경                           |
| `build`    | 빌드 시스템 변경                          |
